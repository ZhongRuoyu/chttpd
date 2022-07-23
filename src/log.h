#ifndef CHTTPD_LOG_H_
#define CHTTPD_LOG_H_

#include "socket.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

void Info(const char *format, ...) __attribute__((format(printf, 1, 2)));

void Warning(const char *format, ...) __attribute__((format(printf, 1, 2)));

void Error(const char *format, ...) __attribute__((format(printf, 1, 2)));

void Fatal(const char *format, ...) __attribute__((format(printf, 1, 2)));

void LogRequestLine(const SocketAddress *from_addr, const char *request_line);

#endif  // CHTTPD_LOG_H_
