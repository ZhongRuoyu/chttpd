#include "datetime.h"

#include <stddef.h>
#include <stdio.h>
#include <time.h>

static const char *kWeekdays[] = {"Sun", "Mon", "Tue", "Wed",
                                  "Thu", "Fri", "Sat"};

static const char *kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int GetDateHeader(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    int n = 0;
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n, "Date: ");
    }
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n,
                      "%s, %02d %s %04d %02d:%02d:%02d GMT",
                      kWeekdays[tm.tm_wday], tm.tm_mday, kMonths[tm.tm_mon],
                      1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n, "\r\n");
    }
    return n;
}
