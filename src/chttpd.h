#ifndef CHTTPD_CHTTPD_H_
#define CHTTPD_CHTTPD_H_

#include <sys/socket.h>

#include "socket.h"

#define BACKLOG SOMAXCONN

#define BUFFER_SIZE 4096

typedef struct {
    const char *host;
    const char *index;
    const char *port;
    const char *root;
    const char *server;
} Context;

int ServeRequest(const Context *context, int connection,
                 const SocketAddress *from_addr);

#endif  // CHTTPD_CHTTPD_H_
