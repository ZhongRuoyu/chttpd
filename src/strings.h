#ifndef CHTTPD_STRINGS_H_
#define CHTTPD_STRINGS_H_

#include <stddef.h>

size_t CopyString(char *dest, const char *src, size_t count);

size_t TrimString(char *dest, const char *src, size_t count);

size_t GetNextToken(const char *str, char *buffer, size_t buffer_size);

#endif  // CHTTPD_STRINGS_H_
