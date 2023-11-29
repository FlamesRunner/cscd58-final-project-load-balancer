#include "hdrs/HealthReporter.hpp"
#include <cstring>
#include <getopt.h>
#include <iostream>
using namespace std;

void print_usage(FILE *fd, const char *prg_name) {
  fprintf(fd, "Usage: %s <port> <enc_key>\n", prg_name);
  fprintf(fd, "\t<port>: Required. Specifies the port to listen on.\n");
  fprintf(fd, "\t<enc_key>: Required. Specifies the encryption key to use (16 "
              "chars).\n");
#ifdef PRODUCTION
  fprintf(fd, "Build: production\n");
#endif
#ifdef DEBUG
  fprintf(fd, "Build: with debug symbols\n");
#endif
}

void init_health_reporter(int port, const char *enc_key) {
  HealthReporter hr(port, std::string(enc_key));
  hr.start();
}

int main(int argc, char **argv) {
  if (argc != 3) {
    print_usage(stderr, argv[0]);
    return 1;
  }

  // Initialize random seed (though we should swap this out with a better prng)
  srand(time(NULL));

  int port;
  char *endptr;
  port = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "Invalid port: %s\n", argv[1]);
    print_usage(stderr, argv[0]);
    return 1;
  }

  // Check if the encryption key is exactly 16 characters
  if (strlen(argv[2]) != 16) {
    fprintf(stderr, "Invalid encryption key: %s\n", argv[2]);
    print_usage(stderr, argv[0]);
    return 1;
  }

  init_health_reporter(port, argv[2]);

  return 0;
}
