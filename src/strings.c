#include "strings.h"

#include <ctype.h>
#include <stddef.h>

size_t CopyString(char *dest, const char *src, size_t count) {
    if (count == 0) {
        return 0;
    }
    size_t bytes_copied = 0;
    while (src[bytes_copied] != '\0' && bytes_copied + 1 < count) {
        dest[bytes_copied] = src[bytes_copied];
        ++bytes_copied;
    }
    dest[bytes_copied] = '\0';
    return bytes_copied;
}

size_t TrimString(char *dest, const char *src, size_t count) {
    if (count == 0) {
        return 0;
    }
    size_t bytes_read = 0;
    size_t bytes_copied = 0;
    size_t last_non_space = -1;
    while (src[bytes_read] != '\0' && bytes_copied + 1 < count) {
        if (!isspace(src[bytes_read])) {
            dest[bytes_copied] = src[bytes_read];
            last_non_space = bytes_copied;
            ++bytes_copied;
        }
        ++bytes_read;
    }
    dest[bytes_copied] = '\0';
    return bytes_copied;
}

size_t GetNextToken(const char *str, char *buffer, size_t buffer_size) {
    size_t p = 0;
    size_t token_length = 0;
    while (token_length + 1 < buffer_size && str[p] != '\0' &&
           !isspace(str[p])) {
        buffer[token_length++] = str[p++];
    }
    buffer[token_length] = '\0';
    return token_length;
}
