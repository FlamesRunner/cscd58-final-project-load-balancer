#include "hdrs/LoadBalancerState.hpp"
#include "../global-hdrs/HealthReporterDS.hpp"
#include "hdrs/plusaes.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <thread>
using namespace std;

LoadBalancerState::LoadBalancerState(LoadBalancerConfiguration config) {
  this->config = config;

  // Initialize nodes
  for (NodeConfiguration &node_config : this->config.nodes) {
    shared_ptr<NodeState> node_state = make_shared<NodeState>(node_config);
    node_state->set_status(NODE_STATUS_DOWN);
    node_state->set_last_checked(0);
    this->nodes.push_back(node_state);
  }

  // Initialize load balancer strategy
  if (config.balancer_algorithm == "ROUND_ROBIN") {
    this->load_balancer_strategy = new LBRoundRobin();
  } else if (config.balancer_algorithm == "RANDOM") {
    this->load_balancer_strategy = new LBRandom();
  } else if (config.balancer_algorithm == "RESOURCE") {
    this->load_balancer_strategy = new LBResource();
  } else {
    cerr << "Invalid load balancer algorithm: " << config.balancer_algorithm
         << endl;
    exit(1);
  }
}

void LoadBalancerState::lb_rt_thread(shared_ptr<NodeState> node_ptr) {
  NodeState &node = *node_ptr;
  while (true) {
    struct sockaddr_in node_address;
    int sockfd = -1;
    node_address.sin_family = AF_INET;
    node_address.sin_port = htons(node.node_config.health_daemon);
    node_address.sin_addr.s_addr = inet_addr(node.node_config.host.c_str());

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
      // Failed to create socket
      node.set_status(NODE_STATUS_DOWN);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }

    int conn =
        connect(sockfd, (struct sockaddr *)&node_address, sizeof(node_address));
    if (conn == -1) {
      // Free socket
      node.set_status(NODE_STATUS_DOWN);
      close(sockfd);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }
    // Connection successful. We attempt to send the first handshake message.
    send(sockfd, HR_HANDSHAKE_MSG_1, strlen(HR_HANDSHAKE_MSG_1) + 1, 0);

    // Await a response; we're looking for a message with HR_HANDSHAKE_MSG_2
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read;
    bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0) {
      // Connection failed.
      node.set_status(NODE_STATUS_DOWN);
      close(sockfd);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }

    // Check if the buffer matches
    if (strncmp(buffer, HR_HANDSHAKE_MSG_2, strlen(HR_HANDSHAKE_MSG_2)) != 0) {
      // Invalid handshake received.
      node.set_status(NODE_STATUS_DOWN);
      close(sockfd);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }

    // Send HANDSHAKE_MSG_3
    send(sockfd, HR_HANDSHAKE_MSG_3, strlen(HR_HANDSHAKE_MSG_3) + 1, 0);

    // Await a response; we're looking for a message with HR_HANDSHAKE_MSG_4
    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0) {
      // Connection failed.
      node.set_status(NODE_STATUS_DOWN);
      close(sockfd);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }

    // Check if the buffer is prefixed with HR_HANDSHAKE_MSG_4
    if (strncmp(buffer, HR_HANDSHAKE_MSG_4, strlen(HR_HANDSHAKE_MSG_4)) != 0) {
      // Invalid handshake received.
      node.set_status(NODE_STATUS_DOWN);
      close(sockfd);
      this_thread::sleep_for(chrono::seconds(1));
      continue;
    }

    // Parse the IV
    string iv_str = string(buffer).substr(strlen(HR_HANDSHAKE_MSG_4));
    unsigned char iv[16];
    for (int i = 0; i < 16; i++) {
      iv[i] = iv_str[i];
    }

#ifdef DEBUG
    cout << "Connection established with node " << node.node_config.name
         << " with IV " << iv_str << endl;
#endif

#ifdef PROD
    cout << "Connection established with node " << node.node_config.name
         << endl;
#endif

    while (true) {
      memset(buffer, 0, BUFFER_SIZE);
      bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
      if (bytes_read <= 0) // Connection failed.
        break;

      // Attempt to decrypt the payload.
      // The following are separated by newlines:
      // 1st line: payload
      // 2nd line: payload length
      string payload = string(buffer);
      size_t newline_pos = payload.find("\n");
      if (newline_pos == string::npos) {
        cout << "Invalid payload received. Ignoring." << endl;
        // Invalid payload received.
        continue;
      }

      // Extract the payload length
      string payload_len_str = payload.substr(newline_pos + 1);
      int payload_len = stoi(payload_len_str);
      if (payload_len <= 0) {
        // Invalid payload length received.
        cout << "Invalid payload length received (" << payload_len
             << "). Ignoring." << endl;
        node.set_status(NODE_STATUS_DOWN);
        continue;
      }

      // Base64 decode the payload
      char *decoded_payload = new char[payload.size() * 2];
      memset(decoded_payload, 0, payload.size() * 2);
      Base64decode(decoded_payload, payload.c_str());

      // Decrypt the payload
      vector<unsigned char> decrypted_payload(payload_len);
      unsigned long padded_size;
      plusaes::decrypt_cbc((unsigned char *)decoded_payload, payload_len,
                           (unsigned char *)this->config.enc_key.c_str(),
                           this->config.enc_key.size(), &iv,
                           &decrypted_payload[0], decrypted_payload.size(),
                           &padded_size);

      // Deserialize the payload
      HealthReport_t hr = HealthReportSerialzer::deserialize(
          string(decrypted_payload.begin(), decrypted_payload.end()));

      // Update the node state
      node.cpu_usage = hr.cpu_load;
      node.mem_usage = hr.mem_load;
      node.set_last_checked(time(NULL));

#ifdef DEBUG
      cout << "Received health report from node " << node.node_config.name
           << ": " << node.cpu_usage << ", " << node.mem_usage << endl;
#endif

      // Set node as up
      node.set_status(NODE_STATUS_UP);
    }

    node.set_status(NODE_STATUS_DOWN);
    close(sockfd);
  }
}

void LoadBalancerState::invalidator_thread(
    vector<shared_ptr<NodeState>> nodes) {
  while (true) {
    // Invalidate nodes that haven't been checked in a while
    for (shared_ptr<NodeState> node_ptr : nodes) {
      NodeState &node = *node_ptr;
      if (node.get_status() == NODE_STATUS_UP) {
        if (time(NULL) - node.get_last_checked() >
            this->get_config().time_until_node_down) {
          cout << "No report received from node " << node.node_config.name
               << " in the last 5 seconds. Marking as down." << endl;
          node.set_status(NODE_STATUS_DOWN);
        }
      }
    }

    this_thread::sleep_for(chrono::seconds(1));
  }
}

void LoadBalancerState::start_rt_checks(void) {
  // For each node, read the LB connector port

  for (shared_ptr<NodeState> &node : this->getNodes()) {
    cerr << "Starting real-time health check for node "
         << node->node_config.name << "..." << endl;
    thread rt_check_thread(&LoadBalancerState::lb_rt_thread, this, node);
    rt_check_thread.detach();
  }

  // Start invalidation thread
  thread invalidator_thread(&LoadBalancerState::invalidator_thread, this,
                            this->getNodes());
  invalidator_thread.detach();
}

LoadBalancerConfiguration LoadBalancerState::get_config(void) {
  return this->config;
}

NodeStatus NodeState::get_status(void) { return this->status; }

int NodeState::get_last_checked(void) { return this->last_checked; }

void NodeState::set_status(NodeStatus status) { this->status = status; }

bool LoadBalancerState::ping_health_check(NodeState &node) {
  // Ping the node (need to fix so it isn't platform dependent).
  // Don't output anything to stdout
  int ping_result = system(
      ("ping -c 1 -W 5 " + node.node_config.host + " > /dev/null").c_str());
#ifdef DEBUG
  cout << "Pinging node '" << node.node_config.name << "' at "
       << node.node_config.host << "... ";
  cout << "Ping result: " << ping_result << endl;
#endif

  // Check the result
  if (ping_result == 0) {
// Node is up
#ifdef DEBUG
    cout << "Node " << node.node_config.name << " is up!" << endl;
#endif
    return true;
  } else {
// Node is down
#ifdef DEBUG
    cout << "Node " << node.node_config.name << " is down!" << endl;
#endif
    return false;
  }
}

bool LoadBalancerState::tcp_health_check(NodeState &node) {
  // Stub: need to implement
  return true;
}

void LoadBalancerState::run_health_checks(void) {
  // Run timed health checks. This is only used
  // for ICMP pings, as the resource health
  // check is timed by the health reporter service.
  bool icmp_check = (this->config.balancer_algorithm != "RESOURCE");
  for (shared_ptr<NodeState> &node_ptr : this->nodes) {
    NodeState &node = *node_ptr;
    bool status = false;
    if (icmp_check) {
      status = this->ping_health_check(node);
      node.set_status(status ? NODE_STATUS_UP : NODE_STATUS_DOWN);
    }
  }
}

NodeState::NodeState(NodeConfiguration &node_config) {
  this->node_config = node_config;
  this->cpu_usage = 0;
  this->mem_usage = 0;
  this->status = NODE_STATUS_DOWN;
}

void NodeState::set_last_checked(int last_checked) {
  this->last_checked = last_checked;
}

std::vector<std::shared_ptr<NodeState>> LoadBalancerState::getNodes() {
  return this->nodes;
}