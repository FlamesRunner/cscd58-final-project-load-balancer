#include "hdrs/LoadBalancerConfiguration.hpp"
#include "hdrs/json.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using json = nlohmann::json;

/**
 * Reads the configuration file and returns a LoadBalancerConfiguration object
 *
 * @param config_file Location of the configuration file
 */
LoadBalancerConfiguration &
LoadBalancerConfiguration::read_config(const char *config_file) {
  cout << "Reading configuration file " << config_file << endl;
  LoadBalancerConfiguration &config = *(new LoadBalancerConfiguration());

  // Read configuration file as JSON
  ifstream config_stream(config_file);
  json config_json;
  config_stream >> config_json;

  // Parse configuration file
  config.max_queued_connections = config_json["max_queued_connections"];
  config.balancer_algorithm = config_json["balancer_algorithm"];
  config.listener_port = config_json["listener_port"];
  config.connection_type = config_json["connection_type"];
  config.enc_key = config_json["enc_key"];
  config.nodes = vector<NodeConfiguration>();
  config.health_check_interval = config_json["health_check_interval"];
  config.time_until_node_down = config_json["time_until_node_down"];

  // Parse nodes
  json nodes_json = config_json["nodes"];

  for (json::iterator it = nodes_json.begin(); it != nodes_json.end(); ++it) {
    NodeConfiguration node;
    node.name = it.value()["name"];
    node.host = it.value()["host"];
    node.target_port = it.value()["target_port"];
    node.health_daemon = it.value()["health_daemon"];
    node.weight = it.value()["weight"];
    config.nodes.push_back(node);

    assert(node.name.length() > 0);
    assert(node.host.length() > 0);
    assert(node.target_port > 0);
    assert(node.health_daemon > 0);
    assert(node.weight >= 0);
  }

  assert(config.max_queued_connections > 0);
  assert(config.listener_port > 0);
  assert(config.enc_key.length() > 0);
  assert(config.nodes.size() > 0);
  assert(config.health_check_interval >= 15);
  assert(config.time_until_node_down >= 1);

  return config;
}