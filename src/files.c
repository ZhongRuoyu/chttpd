#include "files.h"

#include <stdlib.h>
#include <strings.h>

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
