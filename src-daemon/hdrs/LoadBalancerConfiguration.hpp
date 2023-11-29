#ifndef LB_CONF_HDR_DECL
#include "../../global-hdrs/HealthReporterDS.hpp"
#include <iostream>
#include <string>
#include <vector>

class NodeConfiguration {
public:
  std::string name;
  std::string host;
  int target_port;
  int health_daemon;
  int weight;
};

class LoadBalancerConfiguration {
public:
  LoadBalancerConfiguration(){};
  int max_queued_connections;
  std::string balancer_algorithm;
  int listener_port;
  std::string connection_type;
  std::string enc_key;
  std::vector<NodeConfiguration> nodes;
  int health_check_interval;
  int time_until_node_down;

  // Configuration reader
  static LoadBalancerConfiguration &read_config(const char *config_file);
};

#define LB_CONF_HDR_DECL
#endif
