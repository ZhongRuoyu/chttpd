#include "cmdline.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

extern const char *GetVersion();

void Usage(FILE *out) {
    fprintf(
        out,
        "Usage: chttpd [options]\n"
        "Options:\n"
        "  --help                    Report usage information\n"
        "  -v, --version             Report version information\n"
        "  -d, --daemon              Run as daemon\n"
        "  -h HOST, --host HOST      Serve only requests to HOST\n"
        "  -i INDEX, --index INDEX   Set index file (default: index.html)\n"
        "  -l FILE, --log FILE       Set log output path\n"
        "  -p PORT, --port PORT      Set port number to listen to\n"
        "                              (default: 80)\n"
        "  -r ROOT, --root ROOT      Set root directory to serve (default: .)\n"
        "  --server SERVER           Set server name to show in response\n"
        "                              header (default: chttpd)\n"
        "");
}

void Version(FILE *out) {
    fprintf(out,
            "%s\n"
            "",
            GetVersion());
}

static int ReadFlagWithoutDash(int argc, char **argv, int i, size_t offset,
                               const char *name) {
    if (argv[i][offset] == '\0') {
        return 0;
    }
    if (strcmp(argv[i] + offset, name) == 0) {
        return 1;
    }
    return 0;
}

static int ReadFlag(int argc, char **argv, int i, const char *name) {
    if (argv[i][0] == '\0') {
        return 0;
    }
    {
        if (argv[i][0] != '-') {
            return 0;
        }
        int arg_adv = ReadFlagWithoutDash(argc, argv, i, 1, name);
        if (arg_adv != 0) {
            return arg_adv;
        }
    }
    {
        if (argv[i][1] != '-') {
            return 0;
        }
        int arg_adv = ReadFlagWithoutDash(argc, argv, i, 2, name);
        if (arg_adv != 0) {
            return arg_adv;
        }
    }
    return 0;
}

static int ReadArgWithoutDash(int argc, char **argv, int i, size_t offset,
                              const char *name, const char **value) {
    if (argv[i][offset] == '\0') {
        return 0;
    }
    if (strncmp(argv[i] + offset, name, strlen(name)) != 0) {
        return 0;
    }
    if (argv[i][offset + strlen(name)] == '=') {
        *value = argv[i] + offset + strlen(name) + 1;
        return 1;
    }
    if (argv[i][offset + strlen(name)] != '\0') {
        return 0;
    }
    if (i + 1 >= argc) {
        Fatal(NULL, "option %s: argument missing", argv[i]);
    }
    *value = argv[i + 1];
    return 2;
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

void ParseArgs(int argc, char **argv, Args *args) {
    for (int i = 0, arg_adv; i < argc;) {
        if ((arg_adv = ReadFlag(argc, argv, i, "help"))) {
            args->help = true;
            i += arg_adv;
        } else if ((arg_adv = ReadFlag(argc, argv, i, "v")) ||
                   (arg_adv = ReadFlag(argc, argv, i, "version"))) {
            args->version = true;
            i += arg_adv;
        } else if ((arg_adv = ReadFlag(argc, argv, i, "d")) ||
                   (arg_adv = ReadFlag(argc, argv, i, "daemon"))) {
            args->daemon = true;
            i += arg_adv;
        } else if ((arg_adv = ReadArg(argc, argv, i, "h", &args->host)) ||
                   (arg_adv = ReadArg(argc, argv, i, "host", &args->host))) {
            i += arg_adv;
        } else if ((arg_adv = ReadArg(argc, argv, i, "i", &args->index)) ||
                   (arg_adv = ReadArg(argc, argv, i, "index", &args->index))) {
            i += arg_adv;
        } else if ((arg_adv = ReadArg(argc, argv, i, "l", &args->log)) ||
                   (arg_adv = ReadArg(argc, argv, i, "log", &args->log))) {
            i += arg_adv;
        } else if ((arg_adv = ReadArg(argc, argv, i, "p", &args->port)) ||
                   (arg_adv = ReadArg(argc, argv, i, "port", &args->port))) {
            i += arg_adv;
        } else if ((arg_adv = ReadArg(argc, argv, i, "r", &args->root)) ||
                   (arg_adv = ReadArg(argc, argv, i, "root", &args->root))) {
            i += arg_adv;
        } else if ((arg_adv =
                        ReadArg(argc, argv, i, "server", &args->server))) {
            i += arg_adv;
        } else {
            Fatal(NULL, "unknown command line option: %s", argv[i]);
        }
    }
}
