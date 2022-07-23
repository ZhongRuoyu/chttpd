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

size_t GetLineFromConnection(int connection, char *buffer, size_t buffer_size);

int SendFile(int connection, FILE *file);

#endif  // CHTTPD_SOCKET_H_
