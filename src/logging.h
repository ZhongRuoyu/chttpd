#ifndef CHTTPD_LOGGING_H_
#define CHTTPD_LOGGING_H_

#include <netinet/in.h>

void LogRequestLine(const char *from_addr_ip, in_port_t from_addr_port,
                    const char *request_line);

#endif  // CHTTPD_LOGGING_H_
