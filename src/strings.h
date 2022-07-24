#ifndef CHTTPD_STRINGS_H_
#define CHTTPD_STRINGS_H_

#include <stddef.h>

#ifndef __GNUC__
#define __attribute__(x)
#endif

char *Format(const char *format, ...) __attribute__((format(printf, 1, 2)));

size_t CopyString(char *dest, const char *src, size_t count);

char *TrimString(const char *str, size_t *trimmed_length);

char *GetNextToken(const char *str, size_t *token_length);

int SendToConnection(int connection, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

char *GetLineFromConnection(int connection, size_t *line_length);

#endif  // CHTTPD_STRINGS_H_
