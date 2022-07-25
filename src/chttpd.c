#include "chttpd.h"

#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "datetime.h"
#include "files.h"
#include "http.h"
#include "log.h"
#include "socket.h"
#include "strings.h"

static int ProcessRequestLine(const Context *context, int connection,
                              char *request_line, RequestMethod *request_method,
                              char **uri, HTTPVersion *http_version);
static char *GetCommonHeader(const Context *context);
static int ServeFile(const Context *context, int connection, const char *path);
static int SuccessResponseCommonHeader(const Context *context, int connection,
                                       ResponseStatusCode response_status_code);
static int ErrorResponse(const Context *context, int connection,
                         ResponseStatusCode response_status_code);

int ServeRequest(const Context *context, int connection,
                 const SocketAddress *from_addr) {
    size_t request_line_size;
    char *request_line = GetLineFromConnection(connection, &request_line_size);
    if (request_line == NULL) {
        Warning("failed to process request line: %s", strerror(errno));
        if (ErrorResponse(context, connection, kInternalServerError) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }

    RequestMethod request_method;
    char *uri;
    HTTPVersion http_version;
    if (ProcessRequestLine(context, connection, request_line, &request_method,
                           &uri, &http_version) != 0) {
        free(request_line);
        return 1;
    }
    LogRequestLine(from_addr, request_line);
    free(request_line);

    switch (http_version) {
        case kHTTP_1_0:
        case kHTTP_1_1:
            break;
        default:
            if (ErrorResponse(context, connection, kHTTPVersionNotSupported) !=
                0) {
                Warning("failed to send response: %s\n", strerror(errno));
            }
            free(uri);
            return 1;
    }

    char *host_line = NULL;
    for (;;) {
        size_t field_line_length = 0;
        char *field_line =
            GetLineFromConnection(connection, &field_line_length);
        if (field_line == NULL) {
            Warning("failed to process field lines: %s", strerror(errno));
            if (ErrorResponse(context, connection, kInternalServerError) != 0) {
                Warning("failed to send response: %s\n", strerror(errno));
            }
            free(uri);
            return 1;
        }
        if (field_line_length == 0) {
            free(field_line);
            break;
        }
        if (strncasecmp("Host:", field_line, strlen("Host:")) == 0) {
            host_line = field_line;
        } else {
            free(field_line);
        }
    }

    if (host_line != NULL) {
        char *host_line_host = TrimString(host_line + strlen("Host:"), NULL);
        char *host_line_port = NULL;
        free(host_line);
        char *port_seperator = strchr(host_line_host, ':');
        if (port_seperator != NULL) {
            *port_seperator = '\0';
            host_line_port = port_seperator + 1;
        }
        if (http_version == kHTTP_1_1 && context->host != NULL) {
            if (strcmp(host_line_host, context->host) != 0 ||
                host_line_port != NULL &&
                    strcmp(host_line_port, context->port) != 0) {
                free(uri);
                free(host_line_host);
                return 1;
            }
        }
        free(host_line_host);
    } else {
        if (http_version == kHTTP_1_1) {
            free(uri);
            return 1;
        }
    }

    switch (request_method) {
        case kGET: {
            char *query_string = strchr(uri, '?');
            if (query_string != NULL) {
                *query_string = '\0';
                ++query_string;
            }
            char *path = Format("%s%s", context->root, uri);
            free(uri);
            if (path == NULL) {
                Warning("failed to process request path: %s", strerror(errno));
                if (ErrorResponse(context, connection, kInternalServerError) !=
                    0) {
                    Warning("failed to send response: %s\n", strerror(errno));
                }
                free(path);
                return 1;
            }
            {
                int path_is_safe_result = PathIsSafe(path, context->root);
                if (path_is_safe_result == -1) {
                    if (errno == ENOENT) {
                        if (ErrorResponse(context, connection, kNotFound) !=
                            0) {
                            Warning("failed to send response: %s\n",
                                    strerror(errno));
                        }
                        free(path);
                    } else {
                        Warning("failed to process request path: %s",
                                strerror(errno));
                        if (ErrorResponse(context, connection,
                                          kInternalServerError) != 0) {
                            Warning("failed to send response: %s\n",
                                    strerror(errno));
                        }
                        free(path);
                    }
                    return 1;
                } else if (path_is_safe_result != 0) {
                    if (ErrorResponse(context, connection, kBadRequest) != 0) {
                        Warning("failed to send response: %s\n",
                                strerror(errno));
                    }
                    free(path);
                    return 1;
                }
            }
            if (path[strlen(path) - 1] == '/') {
                char *concatenated = Format("%s%s", path, context->index);
                free(path);
                if (concatenated == NULL) {
                    Warning("failed to process request path: %s",
                            strerror(errno));
                    if (ErrorResponse(context, connection,
                                      kInternalServerError) != 0) {
                        Warning("failed to send response: %s\n",
                                strerror(errno));
                    }
                    return 1;
                }
                path = concatenated;
            }
            int serve_result = ServeFile(context, connection, path);
            free(path);
            return serve_result;
        }
        default: {
            if (ErrorResponse(context, connection, kBadRequest) != 0) {
                Warning("failed to send response: %s\n", strerror(errno));
            }
            free(uri);
            return 1;
        }
    }
}

static int ProcessRequestLine(const Context *context, int connection,
                              char *request_line, RequestMethod *request_method,
                              char **uri, HTTPVersion *http_version) {
    size_t request_method_string_length;
    char *request_method_string =
        GetNextToken(request_line, &request_method_string_length);
    if (request_method_string == NULL) {
        Warning("failed to process request method: %s", strerror(errno));
        if (ErrorResponse(context, connection, kInternalServerError) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }
    if (request_method_string_length == 0) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }
    *request_method = GetRequestMethod(request_method_string);
    free(request_method_string);
    if (*request_method == 0) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }

    if (!isspace(request_line[request_method_string_length])) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }

    size_t uri_length;
    *uri = GetNextToken(request_line + request_method_string_length + 1,
                        &uri_length);
    if (uri == NULL) {
        Warning("failed to process URI: %s", strerror(errno));
        if (ErrorResponse(context, connection, kInternalServerError) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }
    if (uri_length == 0) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }

    if (!isspace(request_line[request_method_string_length + 1 + uri_length])) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }

    size_t http_version_string_length;
    char *http_version_string = GetNextToken(
        request_line + request_method_string_length + 1 + uri_length + 1,
        &http_version_string_length);
    if (uri == NULL) {
        Warning("failed to process HTTP version: %s", strerror(errno));
        if (ErrorResponse(context, connection, kInternalServerError) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }
    if (http_version_string_length == 0) {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }
    *http_version = GetHTTPVersion(http_version_string);
    free(http_version_string);
    if (*http_version == 0) {
        Warning("failed to process URI: %s", strerror(errno));
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }

    if (request_line[request_method_string_length + 1 + uri_length + 1 +
                     http_version_string_length] != '\0') {
        if (ErrorResponse(context, connection, kBadRequest) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        free(*uri);
        return 1;
    }

    return 0;
}

static char *GetCommonHeader(const Context *context) {
    char *date_header = GetDateHeader();
    if (date_header == NULL) {
        return NULL;
    }
    char *result = Format(
        "%s"
        "Server: %s\r\n",
        date_header, context->server);
    free(date_header);
    if (result == NULL) {
        return NULL;
    }
    return result;
}

static int ServeFile(const Context *context, int connection, const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        if (ErrorResponse(context, connection, kNotFound) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
        }
        return 1;
    }
    if (SuccessResponseCommonHeader(context, connection, kOK) != 0) {
        Warning("failed to send response: %s\n", strerror(errno));
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    if (SendToConnection(connection, "Content-Length: %ld\r\n", file_size) !=
        0) {
        Warning("failed to send response: %s\n", strerror(errno));
        return 1;
    }

    const char *file_extension = strrchr(path, '.');
    const char *content_type = GetContentType(file_extension);
    if (content_type != NULL) {
        if (SendToConnection(connection, "Content-Type: %s\r\n",
                             content_type) != 0) {
            Warning("failed to send response: %s\n", strerror(errno));
            return 1;
        }
    }

    if (SendToConnection(connection, "\r\n") != 0) {
        Warning("failed to send response: %s\n", strerror(errno));
        return 1;
    }

    if (SendFile(connection, file) != 0) {
        Warning("failed to send response: %s", strerror(errno));
        return 1;
    }

    return 0;
}

static int SuccessResponseCommonHeader(
    const Context *context, int connection,
    ResponseStatusCode response_status_code) {
    if (SendToConnection(connection, "%s %s\r\n",
                         GetHTTPVersionString(kHTTP_1_1),
                         GetResponseStatusString(response_status_code)) != 0) {
        return 1;
    }
    char *common_header = GetCommonHeader(context);
    if (common_header == NULL) {
        return 1;
    }
    if (SendToConnection(connection, "%s", common_header) != 0) {
        free(common_header);
        return 1;
    }
    free(common_header);
    return 0;
}

static int ErrorResponse(const Context *context, int connection,
                         ResponseStatusCode response_status_code) {
    if (SendToConnection(connection, "%s %s\r\n",
                         GetHTTPVersionString(kHTTP_1_1),
                         GetResponseStatusString(response_status_code)) != 0) {
        return 1;
    }
    char *common_header = GetCommonHeader(context);
    if (common_header == NULL) {
        return 1;
    }
    if (SendToConnection(connection, "%s", common_header) != 0) {
        free(common_header);
        return 1;
    }
    free(common_header);
    if (SendToConnection(connection, "\r\n") != 0) {
        return 1;
    }
    if (SendToConnection(connection, "%s\n",
                         GetResponseStatusString(response_status_code)) != 0) {
        return 1;
    }
    return 0;
}
