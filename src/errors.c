#include "errors.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
