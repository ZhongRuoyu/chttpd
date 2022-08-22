#include "datetime.h"

#include <time.h>

#include "strings.h"

static const char *const kWeekdays[] = {"Sun", "Mon", "Tue", "Wed",
                                        "Thu", "Fri", "Sat"};

static const char *const kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *GetDateHeader(void) {
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    return Format("Date: %s, %02d %s %04d %02d:%02d:%02d GMT\r\n",
                  kWeekdays[tm->tm_wday], tm->tm_mday, kMonths[tm->tm_mon],
                  1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
}
