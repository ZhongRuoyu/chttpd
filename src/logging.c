#include "logging.h"

#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define LOG_TIME_BUFFER_SIZE 20

static int GetLogTime(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int n = snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d",
                     1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour,
                     tm.tm_min, tm.tm_sec);
    return n;
}

void LogRequestLine(const char *from_addr_ip, in_port_t from_addr_port,
                    const char *request_line) {
    char log_time[LOG_TIME_BUFFER_SIZE];
    GetLogTime(log_time, sizeof log_time);
    printf("[%s [%s]:%d] %s\n", log_time, from_addr_ip, from_addr_port,
           request_line);
}
