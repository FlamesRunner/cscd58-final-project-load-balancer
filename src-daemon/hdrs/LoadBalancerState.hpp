#ifndef LB_BS_HDR_DECL
#include <iostream>
#include <string>
#include <vector>
#include "LoadBalancerConfiguration.hpp"

enum NodeStatus {
    NODE_STATUS_UP,
    NODE_STATUS_DOWN
};

class NodeState {
    public:
        NodeState(NodeConfiguration node_config);
        NodeConfiguration node_config;
        NodeStatus get_status(void);
        int get_last_checked(void);
        void set_last_checked(int last_checked);
        void set_status(NodeStatus status);
    private:
        int last_checked;
        NodeStatus status;
};

class LoadBalancerState {
    public:
        LoadBalancerState(LoadBalancerConfiguration config);
        LoadBalancerConfiguration get_config(void);
        void run_health_checks(void);
    private:
        LoadBalancerConfiguration config;
        std::vector<NodeState> nodes;
};

#define LB_BS_HDR_DECL
#endif 