#include "files.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "chttpd.h"

#define SEND_FILE_BUFFER_SIZE 4096

int PathIsSafe(const char *path, const char *root) {
    char *path_absolute_path = realpath(path, NULL);
    if (path_absolute_path == NULL) {
        return -1;
    }
    char *root_absolute_path = realpath(root, NULL);
    if (root_absolute_path == NULL) {
        free(path_absolute_path);
        return -1;
    }
    size_t root_absolute_path_length = strlen(root_absolute_path);
    if (strncmp(path_absolute_path, root_absolute_path,
                root_absolute_path_length) != 0) {
        free(path_absolute_path);
        free(root_absolute_path);
        return 1;
    }
    if (path_absolute_path[root_absolute_path_length] != '\0' &&
        path_absolute_path[root_absolute_path_length] != '/') {
        free(path_absolute_path);
        free(root_absolute_path);
        return 1;
    }
    free(path_absolute_path);
    free(root_absolute_path);
    return 0;
}

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
