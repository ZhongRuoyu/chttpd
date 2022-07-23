#ifndef CHTTPD_LOGGING_H_
#define CHTTPD_LOGGING_H_

#include "socket.h"

void LogRequestLine(SocketAddress *from_addr, const char *request_line);

#endif  // CHTTPD_LOGGING_H_
