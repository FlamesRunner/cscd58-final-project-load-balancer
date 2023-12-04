#include <iostream>
#include <getopt.h>
#include <cstring>
#include "hdrs/HealthReporter.hpp"
using namespace std;

/**
 * Prints the health reporter usage documentation.
 * 
 * @param fd Where to print the documentation
 * @param prg_name The name of the executable
 * @returns void
*/
void print_usage(FILE *fd, const char *prg_name)
{
    fprintf(fd, "Usage: %s <port> <enc_key>\n", prg_name);
    fprintf(fd, "\t<port>: Required. Specifies the port to listen on.\n");
    fprintf(fd, "\t<enc_key>: Required. Specifies the encryption key to use (16 chars).\n");
#ifdef PRODUCTION
    fprintf(fd, "Build: production\n");
#endif
#ifdef DEBUG
    fprintf(fd, "Build: with debug symbols\n");
#endif
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        print_usage(stderr, argv[0]);
        return 1;
    }

    int port;
    char *endptr;
    port = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid port: %s\n", argv[1]);
        print_usage(stderr, argv[0]);
        return 1;
    }

    // Check if the encryption key is exactly 16 characters
    if (strlen(argv[2]) != 16)
    {
        fprintf(stderr, "Invalid encryption key: %s\n", argv[2]);
        print_usage(stderr, argv[0]);
        return 1;
    }

    HealthReporter hr(port, std::string(argv[2]));
    hr.start();

    return 0;
}
