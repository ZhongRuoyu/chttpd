#include "socket.h"

#include <netinet/in.h>

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
