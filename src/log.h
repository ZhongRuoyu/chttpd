#ifndef CHTTPD_LOG_H_
#define CHTTPD_LOG_H_

#include "chttpd.h"
#include "socket.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

void Info(const Context *context, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

void Warning(const Context *context, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

void Error(const Context *context, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

void Fatal(const Context *context, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

void LogRequestLine(const Context *context, const SocketAddress *from_addr,
                    const char *request_line);

#endif  // CHTTPD_LOG_H_
