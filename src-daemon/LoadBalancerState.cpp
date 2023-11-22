#include <iostream>
#include "hdrs/LoadBalancerState.hpp"
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
    // For now, we just do a ping
    bool icmp_check = (this->config.balancer_algorithm != "RESOURCE");
    for (NodeState &node : this->nodes)
    {
        bool status = false;
        if (icmp_check)
        {
            status = this->ping_health_check(node);
        }
        else
        {
            status = this->tcp_health_check(node);
        }
        node.set_status(status ? NODE_STATUS_UP : NODE_STATUS_DOWN);
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