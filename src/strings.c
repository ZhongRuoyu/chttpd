#include "strings.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

char *Format(const char *format, ...) {
    char *buffer;
    size_t buffer_size;
    FILE *buffer_memstream = open_memstream(&buffer, &buffer_size);
    if (buffer_memstream == NULL) {
        return NULL;
    }

    va_list args;
    va_start(args, format);
    vfprintf(buffer_memstream, format, args);
    va_end(args);
    putc('\0', buffer_memstream);

    fclose(buffer_memstream);
    return buffer;
}

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

char *TrimString(const char *str, size_t *trimmed_length) {
    char *buffer;
    size_t buffer_size;
    FILE *buffer_memstream = open_memstream(&buffer, &buffer_size);
    if (buffer_memstream == NULL) {
        return NULL;
    }

    size_t i = 0;
    while (str[i] != '\0' && isspace(str[i])) {
        ++i;
    }
    while (str[i] != '\0' && !isspace(str[i])) {
        putc(str[i++], buffer_memstream);
    }
    putc('\0', buffer_memstream);

    fclose(buffer_memstream);
    if (trimmed_length != NULL) {
        *trimmed_length = buffer_size - 1;
    }
    return buffer;
}

char *GetNextToken(const char *str, size_t *token_length) {
    char *buffer;
    size_t buffer_size;
    FILE *buffer_memstream = open_memstream(&buffer, &buffer_size);
    if (buffer_memstream == NULL) {
        return NULL;
    }

    for (size_t i = 0; str[i] != '\0' && !isspace(str[i]); ++i) {
        putc(str[i], buffer_memstream);
    }
    putc('\0', buffer_memstream);

    fclose(buffer_memstream);
    if (token_length != NULL) {
        *token_length = buffer_size - 1;
    }
    return buffer;
}

char *GetLineFromConnection(int connection, size_t *line_length) {
    char *buffer;
    size_t buffer_size;
    FILE *buffer_memstream = open_memstream(&buffer, &buffer_size);
    if (buffer_memstream == NULL) {
        return NULL;
    }

    for (;;) {
        char ch;
        ssize_t next = recv(connection, &ch, 1, 0);
        if (next <= 0) {
            break;
        }
        if (ch == '\r') {
            char peek_ch;
            ssize_t peek_next = recv(connection, &peek_ch, 1, MSG_PEEK);
            if (peek_next > 0 && peek_ch == '\n') {
                recv(connection, &ch, 1, 0);
                break;
            }
        }
        putc(ch, buffer_memstream);
    }
    putc('\0', buffer_memstream);

    fclose(buffer_memstream);
    if (line_length != NULL) {
        *line_length = buffer_size - 1;
    }
    return buffer;
}
