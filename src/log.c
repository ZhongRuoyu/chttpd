#include "log.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "socket.h"

#define LOG_TIME_BUFFER_SIZE 20

typedef enum {
    kNoColor = 1,
    kRed = 2,
    kGreen = 3,
    kYellow = 4,
} Color;

static const char *GetColorCode(Color color) {
    switch (color) {
        case kNoColor:
            return "\033[0m";
        case kRed:
            return "\033[1;31m";
        case kGreen:
            return "\033[1;32m";
        case kYellow:
            return "\033[1;33m";
        default:
            return "";
    }
}

static void ColorOutput(FILE *file, Color color, const char *msg) {
    if (isatty(fileno(file))) {
        fprintf(file, "%s%s%s", GetColorCode(color), msg,
                GetColorCode(kNoColor));
    } else {
        fprintf(file, "%s", msg);
    }
}

void Info(const char *format, ...) {
    fprintf(stderr, "chttpd: ");
    ColorOutput(stderr, kGreen, "info: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void Warning(const char *format, ...) {
    fprintf(stderr, "chttpd: ");
    ColorOutput(stderr, kYellow, "warning: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void Error(const char *format, ...) {
    fprintf(stderr, "chttpd: ");
    ColorOutput(stderr, kRed, "error: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void Fatal(const char *format, ...) {
    fprintf(stderr, "chttpd: ");
    ColorOutput(stderr, kRed, "fatal: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static int GetLogTime(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int n = snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d",
                     1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour,
                     tm.tm_min, tm.tm_sec);
    return n;
}

void LogRequestLine(SocketAddress *from_addr, const char *request_line) {
    char log_time[LOG_TIME_BUFFER_SIZE];
    GetLogTime(log_time, sizeof log_time);
    printf("[%s [%s]:%d] %s\n", log_time, from_addr->ip, from_addr->port,
           request_line);
}
