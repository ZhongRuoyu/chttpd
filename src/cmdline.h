#ifndef CHTTPD_CMDLINE_H_
#define CHTTPD_CMDLINE_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    bool help;
    const char *host;
    const char *port;
    const char *root;
} Args;

void Usage(FILE *out);

void ParseArgs(int argc, char **argv, Args *args);

#endif  // CHTTPD_CMDLINE_H_
