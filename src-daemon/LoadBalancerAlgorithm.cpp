#include "hdrs/LoadBalancerState.hpp"

/**
 * Round robin algorithm constructor
*/
LBRoundRobin::LBRoundRobin()
{
    this->currentNode = 0;
};

/**
 * Chooses a node based on a round robin selection
 * 
 * @param state The current state of the load balancer
 * @return The node ID of the chosen node
*/
int LBRoundRobin::chooseNode(LoadBalancerState state)
{
    // Calculate total weight
    int total = 0;
    int current = 0;
    int nodeId = 0;

    for (NodeState node : state.getNodes())
    {
        if (node.get_status() == NODE_STATUS_UP)
            total += node.node_config.weight;
    }

    if (this->currentNode >= total)
    {
        this->currentNode = 0;
    }

    for (NodeState node : state.getNodes())
    {
        if (node.get_status() == NODE_STATUS_DOWN) {
            continue;
        }

        current += node.node_config.weight;
        if (this->currentNode > current)
        {
            this->currentNode++;
            return nodeId;
        }

        ++nodeId;
    }

    return -1;
}

/**
 * Chooses a node based on a random weighted selection
 * 
 * @param state The current state of the load balancer
 * @return The node ID of the chosen node
*/
int LBRandom::chooseNode(LoadBalancerState state)
{
    // Calculate total weight
    int total = 0;
    std::vector<NodeState> nodes = state.getNodes();
    std::vector<NodeState> upNodes = std::vector<NodeState>();

    for (NodeState node : nodes)
    {
        if (node.get_status() == NODE_STATUS_UP) {
            upNodes.push_back(node);
            total += node.node_config.weight;
        }
    }

    // Generate random number
    int random = rand() % total;

    // Find node
    int current = 0;
    int nodeId = 0;
    for (NodeState node : upNodes)
    {
        current += node.node_config.weight;
        if (random < current)
        {
            return nodeId;
        }

        ++nodeId;
    }

    return -1;
}

/**
 * Chooses a node based on the server's resources
 * 
 * @param state The current state of the load balancer
 * @return The node ID of the chosen node
*/
int LBResource::chooseNode(LoadBalancerState state)
{
    return 0;
}