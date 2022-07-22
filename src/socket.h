#ifndef CHTTPD_SOCKET_H_
#define CHTTPD_SOCKET_H_

#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>

const void *GetInAddr(const struct sockaddr *addr);

in_port_t GetInPort(const struct sockaddr *addr);

size_t GetLineFromConnection(int connection, char *buffer, size_t buffer_size);

int SendFile(int connection, FILE *file);

#endif  // CHTTPD_SOCKET_H_
