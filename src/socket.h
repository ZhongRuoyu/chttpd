#ifndef CHTTPD_SOCKET_H_
#define CHTTPD_SOCKET_H_

#include <netinet/in.h>

const void *GetInAddr(const struct sockaddr *addr);

in_port_t GetInPort(const struct sockaddr *addr);

#endif  // CHTTPD_SOCKET_H_
