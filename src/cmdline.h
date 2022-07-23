#ifndef CHTTPD_CMDLINE_H_
#define CHTTPD_CMDLINE_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    bool help;
    bool version;
    const char *host;
    const char *port;
    const char *root;
    const char *server;
} Args;

void Usage(FILE *out);

void Version(FILE *out);

void ParseArgs(int argc, char **argv, Args *args);

#endif  // CHTTPD_CMDLINE_H_
