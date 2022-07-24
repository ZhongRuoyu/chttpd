#include "files.h"

#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "chttpd.h"

#define SEND_FILE_BUFFER_SIZE 4096

const char *GetContentType(const char *file_extension) {
    if (file_extension == NULL) {
        return NULL;
    }
    if (strcasecmp(file_extension, ".css") == 0) {
        return "text/css";
    } else if (strcasecmp(file_extension, ".html") == 0) {
        return "text/html";
    } else if (strcasecmp(file_extension, ".js") == 0) {
        return "text/javascript";
    } else {
        return NULL;
    }
}

int SendFile(int connection, FILE *file) {
    char buffer[SEND_FILE_BUFFER_SIZE];
    for (;;) {
        size_t bytes_read = fread(buffer, sizeof(char), sizeof buffer, file);
        if (bytes_read == 0) {
            break;
        }
        ssize_t bytes_sent = send(connection, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            return 1;
        }
    }
    return 0;
}
