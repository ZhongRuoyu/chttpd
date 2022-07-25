#ifndef CHTTPD_CMDLINE_H_
#define CHTTPD_CMDLINE_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    bool help;
    bool version;
    bool daemon;
    const char *host;
    const char *index;
    const char *log;
    const char *port;
    const char *root;
    const char *server;
} Args;

void Usage(FILE *out);

void Version(FILE *out);

void ParseArgs(int argc, char **argv, Args *args);

#endif  // CHTTPD_CMDLINE_H_
