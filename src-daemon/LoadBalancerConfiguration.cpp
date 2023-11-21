#include<iostream>
#include <fstream>
#include "hdrs/config.hpp"
#include "hdrs/json.hpp"

using namespace std;
using json = nlohmann::json;

/**
 * Reads the configuration file and returns a LoadBalancerConfiguration object
 * 
 * @param config_file Location of the configuration file
*/
LoadBalancerConfiguration LoadBalancerConfiguration::read_config(const char *config_file) {
    cout << "Reading configuration file " << config_file << endl;
    LoadBalancerConfiguration config;

    // Read configuration file as JSON
    ifstream config_stream(config_file);
    json config_json;
    config_stream >> config_json;
    
    // Parse configuration file
    config.worker_threads = config_json["worker_threads"];
    config.balancer_algorithm = config_json["balancer_algorithm"];
    config.listener_port = config_json["listener_port"];
    config.connection_type = config_json["connection_type"];
    config.enc_key = config_json["enc_key"];
    config.nodes = vector<NodeConfiguration>();
    
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
    }

    assert(config.worker_threads > 0);
    assert(config.listener_port > 0);
    assert(config.enc_key.length() > 0);
    assert(config.nodes.size() > 0);

    #ifdef DEBUG
        cout << "Worker threads: " << config.worker_threads << endl;
        cout << "Balancer algorithm: " << config.balancer_algorithm << endl;
        cout << "Listener port: " << config.listener_port << endl;
        cout << "Connection type: " << config.connection_type << endl;
        cout << "Encryption key: " << config.enc_key << endl;
        cout << "Nodes: " << endl;
        for (NodeConfiguration node : config.nodes) {
            cout << "\tName: " << node.name << endl;
            cout << "\tHost: " << node.host << endl;
            cout << "\tTarget port: " << node.target_port << endl;
            cout << "\tHealth daemon: " << node.health_daemon << endl;
            cout << "\tWeight: " << node.weight << endl << endl;
        }
    #endif

    return config;
}