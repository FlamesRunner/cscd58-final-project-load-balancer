#include<iostream>
#include<string>
#include<vector>
#ifndef HDR_DECL

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
        int worker_threads;
        std::string balancer_algorithm;
        int listener_port;
        std::string connection_type;
        std::string enc_key;
        std::vector<NodeConfiguration> nodes;
        // Configuration reader
        static LoadBalancerConfiguration read_config(const char *config_file);
};

#define HDR_DECL
#endif
