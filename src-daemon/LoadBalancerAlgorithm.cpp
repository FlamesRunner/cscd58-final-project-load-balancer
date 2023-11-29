#include "hdrs/LoadBalancerState.hpp"
#include <climits>

/**
 * Round robin algorithm constructor
 */
LBRoundRobin::LBRoundRobin() { this->currentNode = 0; };

/**
 * Chooses a node based on a round robin selection
 *
 * @param state The current state of the load balancer
 * @return The node ID of the chosen node
 */
int LBRoundRobin::chooseNode(LoadBalancerState &state) {
  // Calculate total weight
  int total = 0;
  int current = 0;
  int nodeId = 0;

  for (std::shared_ptr<NodeState> &node_ptr : state.getNodes()) {
    NodeState node = *node_ptr;
    if (node.get_status() == NODE_STATUS_UP) {
      total += node.node_config.weight;
    }
  }

  if (this->currentNode >= total) {
    this->currentNode = 0;
  }

  /*
      Essentially, we do the following:
      [0, weight1) = node1
      [weight1, weight2) = node2
      ...
  */
  for (std::shared_ptr<NodeState> &node_ptr : state.getNodes()) {
    NodeState node = *node_ptr;
    if (node.get_status() == NODE_STATUS_UP) {
      current += node.node_config.weight;
      if (this->currentNode < current) {
        this->currentNode++;
        return nodeId;
      }
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
int LBRandom::chooseNode(LoadBalancerState &state) {
  // Calculate total weight
  int total = 0;
  std::vector<std::shared_ptr<NodeState>> node_ptrs = state.getNodes();
  std::vector<NodeState> upNodes = std::vector<NodeState>();

  for (std::shared_ptr<NodeState> &node_ptr : node_ptrs) {
    NodeState node = *node_ptr;
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
  for (NodeState node : upNodes) {
    current += node.node_config.weight;
    if (random < current) {
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
int LBResource::chooseNode(LoadBalancerState &state) {
  // Calculate score for each node. The node with the lowest score is chosen.
  // score(cpu_load, mem_usage, weight) = (cpu_load * 100 + mem_usage) * (1 /
  // weight)
  int lowest_score = INT_MAX;
  int lowest_score_node_id = -1;
  int node_id = 0;

  std::vector<std::shared_ptr<NodeState>> node_ptrs = state.getNodes();

  for (std::shared_ptr<NodeState> &node_ptr : node_ptrs) {
    NodeState node = *node_ptr;
    std::cout << node.cpu_usage << " " << node.mem_usage << " "
              << node.node_config.weight << std::endl;
    if (node.get_status() == NODE_STATUS_UP) {
      int score = (int)((node.cpu_usage * 100 + node.mem_usage) *
                        (1.0 / node.node_config.weight));
      if (score < lowest_score) {
        lowest_score = score;
        lowest_score_node_id = node_id;
      }
    }

    ++node_id;
  }

  return lowest_score_node_id;
}