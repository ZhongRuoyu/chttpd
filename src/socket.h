#ifndef CHTTPD_SOCKET_H_
#define CHTTPD_SOCKET_H_

#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    char ip[INET6_ADDRSTRLEN];
    in_port_t port;
} SocketAddress;

SocketAddress GetSocketAddress(const struct sockaddr_storage *addr_storage);

char *GetLineFromConnection(int connection, size_t *line_length);

int SendFile(int connection, FILE *file);

#endif  // CHTTPD_SOCKET_H_
