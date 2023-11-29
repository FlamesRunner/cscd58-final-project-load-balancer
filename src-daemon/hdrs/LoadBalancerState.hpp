#ifndef LB_BS_HDR_DECL
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "LoadBalancerConfiguration.hpp"

enum NodeStatus
{
    NODE_STATUS_UP,
    NODE_STATUS_DOWN
};

class LoadBalancerState;

class LoadBalancerAlgorithm
{
public:
    virtual int chooseNode(LoadBalancerState &state) = 0;
};

class LBRoundRobin : public LoadBalancerAlgorithm
{
public:
    LBRoundRobin();
    int currentNode;
    int chooseNode(LoadBalancerState &state);
};

class LBRandom : public LoadBalancerAlgorithm
{
public:
    int chooseNode(LoadBalancerState &state);
};

class LBResource : public LoadBalancerAlgorithm
{
public:
    int chooseNode(LoadBalancerState &state);
};

class NodeState
{
public:
    NodeState(NodeConfiguration &node_config);
    NodeConfiguration node_config;
    NodeStatus get_status(void);
    int get_last_checked(void);
    void set_last_checked(int last_checked);
    void set_status(NodeStatus status);
    float cpu_usage;
    float mem_usage;

private:
    int last_checked;
    NodeStatus status;
};

class LoadBalancerState
{
public:
    LoadBalancerState(){};
    LoadBalancerState(LoadBalancerConfiguration config);
    LoadBalancerConfiguration get_config(void);
    std::vector<std::shared_ptr<NodeState>> getNodes();
    void run_health_checks(void);
    LoadBalancerAlgorithm *load_balancer_strategy;
    void start_rt_checks(void);
    void lb_rt_thread(std::shared_ptr<NodeState> node);
    void invalidator_thread(std::vector<std::shared_ptr<NodeState>>);

private:
    LoadBalancerConfiguration config;
    std::vector<std::shared_ptr<NodeState>> nodes;
    bool ping_health_check(NodeState &node);
    bool tcp_health_check(NodeState &node);
    unsigned char iv[16];
};

#define LB_BS_HDR_DECL
#endif