#include "cmdline.h"

#include <getopt.h>
#include <stdio.h>

void Usage(FILE *out) {
    fprintf(out,
            "usage: chttpd [options]\n"
            "options:\n"
            "  --help        Report usage information\n"
            "  -h, --host    Set hostname (default: localhost)\n"
            "  -p, --port    Set port number to listen to (default: 80)\n"
            "  -r, --root    Set root directory to be served (default: .)\n");
}

int ParseArguments(int argc, char **argv, Context *context) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"port", required_argument, NULL, 0},
        {"root", required_argument, NULL, 0},
        {NULL, 0, NULL, 0},
    };
    for (int opt, long_options_index;
         (opt = getopt_long_only(argc, argv, ":h:p:r:", long_options,
                                 &long_options_index)) != -1;) {
        switch (opt) {
            case 0:
                switch (long_options_index) {
                    case 0:
                        context->help = true;
                        break;
                    case 1:
                        context->host = optarg;
                        break;
                    case 2:
                        context->port = optarg;
                        break;
                    case 3:
                        context->root = optarg;
                        break;
                    default:
                        break;
                }
                break;
            case 'h':
                context->host = optarg;
                break;
            case 'p':
                context->port = optarg;
                break;
            case 'r':
                context->root = optarg;
                break;
            case ':':
                fprintf(stderr, "option -%c: argument missing\n", optopt);
                return -1;
            case '?':
                fprintf(stderr, "unknown command line option: %s\n",
                        argv[optind - 1]);
                return -1;
            default:
                break;
        }
    }
    return optind;
}
