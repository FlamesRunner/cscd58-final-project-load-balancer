#include <iostream>
#include <getopt.h>

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
    fprintf(fd, "\t-c <location_to_config>: Required. Specifies the configuration file to use.\n"); 
    fprintf(fd, "\t-d: Detached mode (daemon mode)\n");
    #ifdef PRODUCTION
        fprintf(fd, "Build: production\n");
    #endif
    #ifdef DEBUG
        fprintf(fd, "Build: with debug symbols\n");
    #endif
}

/**
 * Main wrapper function. Handles argument parsing and initializes the daemon.
 *
 * @param argc # of arguments passed
 * @param argv array of character pointers to argument values
 */
int main(int argc, char **argv)
{
    char *str_ptr = nullptr;
    for (;;)
    {
        switch (getopt(argc, argv, "c:h")) // note the colon (:) to indicate that 'b' has a parameter and is not a switch
        {
        case 'h':
            print_usage(stdout, argv[0]);
            exit(0);
            break;
        case 'c':
            str_ptr = optarg;
            fprintf(stdout, "arg: %s\n", str_ptr);
            exit(0);
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
}