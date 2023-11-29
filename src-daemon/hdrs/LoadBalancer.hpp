#ifndef LB_HDR_DECL
#include "LoadBalancerConfiguration.hpp"
#include "LoadBalancerState.hpp"
#include <iostream>
#include <string>
#include <vector>

class LoadBalancer {
public:
  LoadBalancer(const LoadBalancerConfiguration config) {
    this->config = config;
  };
  LoadBalancerConfiguration &get_config(void);
  LoadBalancerConfiguration config;
  void start();
  LoadBalancerState get_state(void);
  void scheduled_tasks();

private:
  LoadBalancerState state;
  void listener(void);
  void forward_traffic(int from_socket, int to_socket);
};

#define LB_HDR_DECL
#endif