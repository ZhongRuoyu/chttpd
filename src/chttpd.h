#ifndef CHTTPD_CHTTPD_H_
#define CHTTPD_CHTTPD_H_

#include <sys/socket.h>

#include "socket.h"

#define BACKLOG SOMAXCONN

#define BUFFER_SIZE 16384
#define TOKEN_BUFFER_SIZE 256
#define LINE_BUFFER_SIZE 1024
#define URI_BUFFER_SIZE 8192

typedef struct {
    const char *host;
    const char *port;
    const char *root;
} Context;

int ServeRequest(Context *context, int connection, SocketAddress *from_addr);

#endif  // CHTTPD_CHTTPD_H_
