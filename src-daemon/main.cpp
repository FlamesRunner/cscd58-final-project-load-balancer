#include "hdrs/LoadBalancer.hpp"
#include "hdrs/LoadBalancerConfiguration.hpp"
#include <getopt.h>
#include <iostream>
#include <thread>

using namespace std;

/**
 * Prints to the specified file descriptor usage information
 * for the wrapper.
 *
 * @param fd File descriptor
 * @param prg_name Name of the executable, usually given by argv[0]
 */
void print_usage(FILE *fd, const char *prg_name) {
  fprintf(fd, "Usage: %s <args>\n", prg_name);
  fprintf(fd, "\t-h: This usage information\n");
  fprintf(fd, "\t-c <location_to_config>: Required. Specifies the "
              "configuration file to use.\n");
#ifdef PRODUCTION
  fprintf(fd, "Build: production\n");
#endif
#ifdef DEBUG
  fprintf(fd, "Build: with debug symbols\n");
#endif
}

/**
 * Initializes the daemon. This includes:
 * - Reading the configuration file
 * - Initializing the logger
 *
 * @param config_file Location of the configuration file
 */
void init_daemon(const char *config_file) {
  // Read configuration file
  LoadBalancerConfiguration &config =
      LoadBalancerConfiguration::read_config(config_file);

  cout << "Maximum queued connections: " << config.max_queued_connections
       << endl;
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

  // Start daemon
  LoadBalancer balancer = LoadBalancer(config);
  balancer.start();
}

/**
 * Main wrapper function. Handles argument parsing and initializes the daemon.
 *
 * @param argc # of arguments passed
 * @param argv array of character pointers to argument values
 */
int main(int argc, char **argv) {
  char *config_file = nullptr;

  // Initialize random seed (though we should swap this out with a better prng)
  srand(time(NULL));

  for (;;) {
    switch (
        getopt(argc, argv, "c:h")) // note the colon (:) to indicate that 'b'
                                   // has a parameter and is not a switch
    {
    case 'h':
      print_usage(stdout, argv[0]);
      exit(0);
      break;
    case 'c':
      config_file = optarg;
      break;
    default:
      fprintf(stderr, "Invalid argument options.\n");
      print_usage(stdout, argv[0]);
      exit(1);
      break;
    case -1: // No more arguments to parse
      break;
    }
    break;
  }

  if (config_file == nullptr) {
    fprintf(stderr, "No configuration file specified.\n");
    print_usage(stderr, argv[0]);
    exit(1);
  }

  init_daemon(config_file);
}