#include "hdrs/LoadBalancer.hpp"
#include "hdrs/LoadBalancerConfiguration.hpp"
#include "hdrs/LoadBalancerState.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

void LoadBalancer::scheduled_tasks() {
  while (true) {
    // Run health checks
    this->state.run_health_checks();

    // Sleep for 15 seconds
    this_thread::sleep_for(
        chrono::seconds(this->get_config().health_check_interval));
  }
}

void LoadBalancer::forward_traffic(int from_socket, int to_socket) {
  // Read data from client
  char buffer[BUFFER_SIZE];
  while (true) {
    int bytes_read = recv(from_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0)
      break;

    send(to_socket, buffer, bytes_read, 0);
  }
}

void LoadBalancer::listener() {
  // Set up listener
  errno = 0;

  int listener_socket;
  if (this->get_config().connection_type == "TCP")
    listener_socket = socket(AF_INET, SOCK_STREAM, 0);
  else if (this->get_config().connection_type == "UDP")
    listener_socket = socket(AF_INET, SOCK_DGRAM, 0);

  if (listener_socket < 0) {
    cout << "Error creating listener socket: code " << errno << endl;
    exit(1);
  }

  struct sockaddr_in listener_address;
  listener_address.sin_family = AF_INET;
  listener_address.sin_port = htons(this->get_config().listener_port);
  listener_address.sin_addr.s_addr = INADDR_ANY;

  // Bind listener
  errno = 0;
  bind(listener_socket, (struct sockaddr *)&listener_address,
       sizeof(listener_address));
  if (errno != 0) {
    cout << "Error binding listener socket: code " << errno << endl;
    exit(1);
  }

  // Listen
  errno = 0;
  listen(listener_socket, this->get_config().max_queued_connections);
  if (errno != 0) {
    cout << "Error listening on listener socket: code " << errno << endl;
    exit(1);
  }

#ifdef DEBUG
  cout << "Listening on port " << this->get_config().listener_port << endl;
#endif

  while (true) {
    // Accept connection
    errno = 0;
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int connection_socket =
        accept(listener_socket, (struct sockaddr *)&client_address,
               &client_address_size);

    if (connection_socket < 0) {
#ifdef DEBUG
      cout << "Error accepting connection: code " << errno << endl;
#endif
      sleep(1);
      continue;
    }

    // Establish connection with node
    struct sockaddr_in node_address;
    node_address.sin_family = AF_INET;

#ifdef DEBUG
    cout << "Connection from " << inet_ntoa(client_address.sin_addr) << ":"
         << ntohs(client_address.sin_port) << endl;
#endif

    // Choose node
    LoadBalancerAlgorithm *algo = this->state.load_balancer_strategy;
    int node_id = algo->chooseNode(this->state);
    if (node_id == -1) {
#ifdef DEBUG
      cout << "No nodes available" << endl;
#endif
      close(connection_socket);
      continue;
    }

#ifdef DEBUG
    cout << "Chose node " << node_id << endl;
#endif

    node_address.sin_port =
        htons(this->get_config().nodes[node_id].target_port);
    node_address.sin_addr.s_addr =
        inet_addr(this->get_config().nodes[node_id].host.c_str());

    int node_socket;
    if (this->get_config().connection_type == "TCP")
      node_socket = socket(AF_INET, SOCK_STREAM, 0);
    else if (this->get_config().connection_type == "UDP")
      node_socket = socket(AF_INET, SOCK_DGRAM, 0);

    connect(node_socket, (struct sockaddr *)&node_address,
            sizeof(node_address));

    // Forward traffic from client to node
    thread forward_client_to_node_thread(&LoadBalancer::forward_traffic, this,
                                         connection_socket, node_socket);
    forward_client_to_node_thread.detach();

    // Forward traffic from node to client
    thread forward_node_to_client_thread(&LoadBalancer::forward_traffic, this,
                                         node_socket, connection_socket);
    forward_node_to_client_thread.detach();
  }
}

void LoadBalancer::start() {
  // Initialize state
  this->state = LoadBalancerState(config);

  // Start scheduled tasks in a new thread
  thread scheduled_tasks_thread(&LoadBalancer::scheduled_tasks, this);
  scheduled_tasks_thread.detach();

  // Check if we need to start resource LB connections
  if (config.balancer_algorithm == "RESOURCE") {
    this->state.start_rt_checks();
  }

  // Start listener
  this->listener();
}

LoadBalancerConfiguration &LoadBalancer::get_config(void) {
  return this->config;
}

LoadBalancerState LoadBalancer::get_state(void) { return this->state; }