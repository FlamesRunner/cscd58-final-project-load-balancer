#include <iostream>
#include "hdrs/LoadBalancerState.hpp"

LoadBalancerState::LoadBalancerState(LoadBalancerConfiguration config) {
    this->config = config;

    // Initialize nodes
    for (NodeConfiguration node_config : config.nodes) {
        NodeState node_state = NodeState(node_config);
        node_state.set_status(NODE_STATUS_DOWN);
        node_state.set_last_checked(0);
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
        std::cerr << "Invalid load balancer algorithm: " << config.balancer_algorithm << std::endl;
        exit(1);
    }
}

LoadBalancerConfiguration LoadBalancerState::get_config(void) {
    return this->config;
}

NodeStatus NodeState::get_status(void) {
    return this->status;
}

int NodeState::get_last_checked(void) {
    return this->last_checked;
}

void NodeState::set_status(NodeStatus status) {
    this->status = status;
}

void LoadBalancerState::run_health_checks(void) {
    // For now, we just do a ping
    for (NodeState node : this->nodes) {
        // Ping the node (need to fix so it isn't platform dependent).
        // Don't output anything to stdout
        int ping_result = system(("ping -c 1 -W 5 " + node.node_config.host + " > /dev/null").c_str());

        // Check the result
        if (ping_result == 0) {
            // Node is up
            node.set_status(NODE_STATUS_UP);
        } else {
            // Node is down
            std::cout << "Node " << node.node_config.name << " is down!" << std::endl;
            node.set_status(NODE_STATUS_DOWN);
        }

        // Update last checked
        node.set_last_checked(time(NULL));
    }
}

NodeState::NodeState(NodeConfiguration node_config) {
    this->node_config = node_config;
}

void NodeState::set_last_checked(int last_checked) {
    this->last_checked = last_checked;
}

std::vector<NodeState> LoadBalancerState::getNodes() {
    return this->nodes;
}