#ifndef CHTTPD_CHTTPD_H_
#define CHTTPD_CHTTPD_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

#include "socket.h"

#define BACKLOG SOMAXCONN

typedef struct {
    bool daemon;
    const char *host;
    const char *index;
    FILE *log;
    const char *port;
    const char *root;
    const char *server;
} Context;

int ServeRequest(const Context *context, int connection,
                 const SocketAddress *from_addr);

#endif  // CHTTPD_CHTTPD_H_
