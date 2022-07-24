#include "socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "chttpd.h"

static const void *GetInAddr(const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return &(((const struct sockaddr_in *)addr)->sin_addr);
    }
    return &(((const struct sockaddr_in6 *)addr)->sin6_addr);
}

static in_port_t GetInPort(const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return ntohs(((const struct sockaddr_in *)addr)->sin_port);
    }
    return ntohs(((const struct sockaddr_in6 *)addr)->sin6_port);
}

SocketAddress GetSocketAddress(const struct sockaddr_storage *addr_storage) {
    SocketAddress socket_address;
    inet_ntop(addr_storage->ss_family,
              GetInAddr((const struct sockaddr *)addr_storage),
              socket_address.ip, sizeof socket_address.ip);
    socket_address.port = GetInPort((const struct sockaddr *)addr_storage);
    return socket_address;
}

char *GetLineFromConnection(int connection, size_t *line_length) {
    char *buffer;
    size_t buffer_size;
    FILE *buffer_memstream = open_memstream(&buffer, &buffer_size);
    if (buffer_memstream == NULL) {
        return NULL;
    }
    for (;;) {
        char ch;
        ssize_t next = recv(connection, &ch, 1, 0);
        if (next <= 0) {
            break;
        }
        if (ch == '\r') {
            char peek_ch;
            ssize_t peek_next = recv(connection, &peek_ch, 1, MSG_PEEK);
            if (peek_next > 0 && peek_ch == '\n') {
                recv(connection, &ch, 1, 0);
                break;
            }
        }
        putc(ch, buffer_memstream);
    }
    putc('\0', buffer_memstream);
    fclose(buffer_memstream);
    if (line_length != NULL) {
        *line_length = buffer_size - 1;
    }
    return buffer;
}

int SendFile(int connection, FILE *file) {
    char buffer[BUFFER_SIZE];
    for (;;) {
        size_t bytes_read = fread(buffer, sizeof(char), sizeof buffer, file);
        if (bytes_read == 0) {
            break;
        }
        ssize_t bytes_sent = send(connection, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            return 1;
        }
    }
    return 0;
}
