#include <iostream>
#include "hdrs/LoadBalancerState.hpp"
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include "hdrs/plusaes.hpp"
#include "../global-hdrs/HealthReporterDS.hpp"
using namespace std;

LoadBalancerState::LoadBalancerState(LoadBalancerConfiguration config)
{
    this->config = config;

    // Initialize nodes
    for (NodeConfiguration node_config : config.nodes)
    {
        NodeState node_state = NodeState(node_config);
        node_state.set_status(NODE_STATUS_DOWN);
        node_state.set_last_checked(0);
        this->nodes.push_back(node_state);
    }

    // Initialize load balancer strategy
    if (config.balancer_algorithm == "ROUND_ROBIN")
    {
        this->load_balancer_strategy = new LBRoundRobin();
    }
    else if (config.balancer_algorithm == "RANDOM")
    {
        this->load_balancer_strategy = new LBRandom();
    }
    else if (config.balancer_algorithm == "RESOURCE")
    {
        this->load_balancer_strategy = new LBResource();
    }
    else
    {
        cerr << "Invalid load balancer algorithm: " << config.balancer_algorithm << endl;
        exit(1);
    }
}

void LoadBalancerState::lb_rt_thread(NodeState &node) {
    // Establish connection (failures re-attempt)
    while(true) {
        struct sockaddr_in node_address;
        int sockfd = -1;
        node_address.sin_family = AF_INET;
        node_address.sin_port = node.node_config.health_daemon;
        node_address.sin_addr.s_addr = inet_addr(node.node_config.host.c_str());
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }

        int conn = connect(sockfd, (struct sockaddr *)&node_address, sizeof(node_address));
        if (conn == -1) {
            // Free socket
            close(sockfd);
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }

        // Connection successful. We attempt to send the first handshake message.
        send(sockfd, HR_HANDSHAKE_MSG_1, strlen(HR_HANDSHAKE_MSG_1) + 1, 0);

        // Await a response; we're looking for a message with HR_HANDSHAKE_MSG_2
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_read;
        bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) {
            // Connection failed.
            close(sockfd);
            continue;
        }

        // Check if the buffer matches
        if (strncmp(buffer, HR_HANDSHAKE_MSG_2, strlen(HR_HANDSHAKE_MSG_2)) != 0) {
            // Invalid handshake received.
            close(sockfd);
            continue;
        }

        while (true)
        {
            memset(buffer, 0, sizeof(buffer));
            bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) // Connection failed.
                break;

            if (strncmp(buffer, HR_HANDSHAKE_MSG_3, strlen(HR_HANDSHAKE_MSG_3) == 0)) {
                // Connection established; ignore msg
                continue;
            }

            // Attempt to decrypt the payload
            // todo

        }

        close(sockfd);
    }   
}

void LoadBalancerState::start_rt_checks(void) {
    // For each node, read the LB connector port
    for (NodeState &node : this->getNodes()) {
        int port = node.node_config.health_daemon;
        thread rt_check_thread(&LoadBalancerState::lb_rt_thread, this, node);
        rt_check_thread.detach();
    }
}

LoadBalancerConfiguration LoadBalancerState::get_config(void)
{
    return this->config;
}

NodeStatus NodeState::get_status(void)
{
    return this->status;
}

int NodeState::get_last_checked(void)
{
    return this->last_checked;
}

void NodeState::set_status(NodeStatus status)
{
    this->status = status;
}

bool LoadBalancerState::ping_health_check(NodeState &node)
{
    // Ping the node (need to fix so it isn't platform dependent).
    // Don't output anything to stdout
    int ping_result = system(("ping -c 1 -W 5 " + node.node_config.host + " > /dev/null").c_str());
#ifdef DEBUG
    cout << "Pinging node '" << node.node_config.name << "' at " << node.node_config.host << "... ";
    cout << "Ping result: " << ping_result << endl;
#endif

    // Check the result
    if (ping_result == 0)
    {
// Node is up
#ifdef DEBUG
        cout << "Node " << node.node_config.name << " is up!" << endl;
#endif
        return true;
    }
    else
    {
// Node is down
#ifdef DEBUG
        cout << "Node " << node.node_config.name << " is down!" << endl;
#endif
        return false;
    }
}

bool LoadBalancerState::tcp_health_check(NodeState &node)
{
    // Stub: need to implement
    return true;
}

void LoadBalancerState::run_health_checks(void)
{
    // Run timed health checks. This is only used
    // for ICMP pings, as the resource health
    // check is timed by the health reporter service.
    bool icmp_check = (this->config.balancer_algorithm != "RESOURCE");
    for (NodeState &node : this->nodes)
    {
        bool status = false;
        if (icmp_check)
        {
            status = this->ping_health_check(node);
            node.set_status(status ? NODE_STATUS_UP : NODE_STATUS_DOWN);
        }
    }
}

NodeState::NodeState(NodeConfiguration node_config)
{
    this->node_config = node_config;
}

void NodeState::set_last_checked(int last_checked)
{
    this->last_checked = last_checked;
}

std::vector<NodeState> LoadBalancerState::getNodes()
{
    return this->nodes;
}