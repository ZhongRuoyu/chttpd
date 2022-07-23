#include "cmdline.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Usage(FILE *out) {
    fprintf(
        out,
        "usage: chttpd [options]\n"
        "options:\n"
        "  --help                Report usage information\n"
        "  -h HOST, --host HOST  Set hostname (default: localhost)\n"
        "  -p PORT, --port PORT  Set port number to listen to (default: 80)\n"
        "  -r ROOT, --root ROOT  Set root directory to serve (default: .)\n");
}

static int ReadFlag(int argc, char **argv, int i, const char *name) {
    if (argv[i][0] == '\0') {
        return 0;
    }
    if (argv[i][0] != '-' || argv[i][1] == '\0') {
        return 0;
    }
    if (strcmp(argv[i] + 1, name) == 0) {
        return 1;
    }
    if (argv[i][1] != '-' || argv[i][2] == '\0') {
        return 0;
    }
    if (strcmp(argv[i] + 2, name) == 0) {
        return 1;
    }
    return 0;
}

static int ReadArgWithoutDash(int argc, char **argv, int i, size_t offset,
                              const char *name, const char **value) {
    if (argv[i][offset] == '\0') {
        return 0;
    }
    if (strncmp(argv[i] + offset, name, strlen(name)) == 0) {
        if (argv[i][offset + strlen(name)] == '=') {
            *value = argv[i] + offset + strlen(name) + 1;
            return 1;
        }
        if (argv[i][offset + strlen(name)] != '\0') {
            return 0;
        }
        if (i + 1 >= argc) {
            fprintf(stderr, "option %s: argument missing\n", argv[i]);
            exit(EXIT_FAILURE);
        }
        *value = argv[i + 1];
        return 2;
    }
    return 0;
}

static int ReadArg(int argc, char **argv, int i, const char *name,
                   const char **value) {
    if (argv[i][0] == '\0') {
        return 0;
    }
    {
        if (argv[i][0] != '-') {
            return 0;
        }
        int arg_adv = ReadArgWithoutDash(argc, argv, i, 1, name, value);
        if (arg_adv != 0) {
            return arg_adv;
        }
    }
    {
        if (argv[i][1] != '-') {
            return 0;
        }
        int arg_adv = ReadArgWithoutDash(argc, argv, i, 2, name, value);
        if (arg_adv != 0) {
            return arg_adv;
        }
    }
    return 0;
}

void ParseArguments(int argc, char **argv, Context *context) {
    for (int i = 0, argc_adv; i < argc;) {
        if ((argc_adv = ReadFlag(argc, argv, i, "help"))) {
            context->help = true;
            i += argc_adv;
        } else if ((argc_adv = ReadArg(argc, argv, i, "h", &context->host)) ||
                   (argc_adv =
                        ReadArg(argc, argv, i, "host", &context->host))) {
            i += argc_adv;
        } else if ((argc_adv = ReadArg(argc, argv, i, "p", &context->port)) ||
                   (argc_adv =
                        ReadArg(argc, argv, i, "port", &context->port))) {
            i += argc_adv;
        } else if ((argc_adv = ReadArg(argc, argv, i, "r", &context->root)) ||
                   (argc_adv =
                        ReadArg(argc, argv, i, "root", &context->root))) {
            i += argc_adv;
        } else {
            fprintf(stderr, "unknown command line option: %s\n", argv[i]);
            Usage(stderr);
            exit(EXIT_FAILURE);
        }
    }
}
