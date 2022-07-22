#include "socket.h"

#include <netinet/in.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>

const void *GetInAddr(const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return &(((const struct sockaddr_in *)addr)->sin_addr);
    }
    return &(((const struct sockaddr_in6 *)addr)->sin6_addr);
}

in_port_t GetInPort(const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return ntohs(((const struct sockaddr_in *)addr)->sin_port);
    }
    return ntohs(((const struct sockaddr_in6 *)addr)->sin6_port);
}

size_t GetLineFromConnection(int connection, char *buffer, size_t buffer_size) {
    if (buffer_size == 0) {
        return 0;
    }
    char ch;
    size_t bytes_read = 0;
    for (char ch; bytes_read + 1 < buffer_size; ++bytes_read) {
        ssize_t n = recv(connection, &ch, 1, 0);
        if (n <= 0) {
            break;
        }
        if (ch == '\r') {
            char next_ch;
            ssize_t next = recv(connection, &next_ch, 1, MSG_PEEK);
            if (next > 0 && next_ch == '\n') {
                n = recv(connection, &ch, 1, 0);
                break;
            }
        }
        buffer[bytes_read] = ch;
    }
    buffer[bytes_read] = '\0';
    return bytes_read;
}
