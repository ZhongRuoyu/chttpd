#ifndef CHTTPD_CONTEXT_H_
#define CHTTPD_CONTEXT_H_

#include <stdbool.h>

typedef struct {
    bool help;
    const char *host;
    const char *port;
    const char *root;
} Context;

Context *GetContext();

#endif  // CHTTPD_CONTEXT_H_
