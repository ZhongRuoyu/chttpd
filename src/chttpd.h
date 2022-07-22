#ifndef CHTTPD_CHTTPD_H_
#define CHTTPD_CHTTPD_H_

#include <netinet/in.h>
#include <stdio.h>

#define BACKLOG SOMAXCONN

#define BUFFER_SIZE 16384
#define TOKEN_BUFFER_SIZE 256
#define LINE_BUFFER_SIZE 1024
#define URI_BUFFER_SIZE 8192

#define INDEX "index.html"

int ServeRequest(const char *host, const char *port, const char *root,
                 int connection, const char *from_addr_ip,
                 in_port_t from_addr_port);

#endif  // CHTTPD_CHTTPD_H_
