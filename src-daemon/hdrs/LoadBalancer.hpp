#ifndef LB_HDR_DECL
#include <iostream>
#include <string>
#include <vector>
#include "LoadBalancerConfiguration.hpp"
#include "LoadBalancerState.hpp"

class LoadBalancer {
    public:
        LoadBalancer(LoadBalancerConfiguration config);
        LoadBalancerConfiguration get_config(void);
        void start();
        LoadBalancerState get_state(void);
        void scheduled_tasks();
    private:
        LoadBalancerConfiguration config;
        LoadBalancerState state = LoadBalancerState(config);
        void listener();
        void forward_traffic(int from_socket, int to_socket);
};

#define LB_HDR_DECL
#endif