#ifndef CHTTPD_ERRORS_H_
#define CHTTPD_ERRORS_H_

#ifndef __GNUC__
#define __attribute__(x)
#endif

void Warning(const char *format, ...) __attribute__((format(printf, 1, 2)));

void Error(const char *format, ...) __attribute__((format(printf, 1, 2)));

void Fatal(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif  // CHTTPD_ERRORS_H_
