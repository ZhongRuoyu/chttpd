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
static size_t GetCommonHeader(const Context *context, char *buffer,
                              size_t buffer_size);
static int ServeFile(const Context *context, int connection, const char *path);
static void SuccessResponse(const Context *context, int connection,
                            ResponseStatusCode code);
static void ErrorResponse(const Context *context, int connection,
                          ResponseStatusCode code);

int ServeRequest(const Context *context, int connection,
                 const SocketAddress *from_addr) {
    size_t request_line_size;
    char *request_line = GetLineFromConnection(connection, &request_line_size);
    if (request_line == NULL) {
        Warning("failed to process request line: %s", strerror(errno));
        ErrorResponse(context, connection, kInternalServerError);
        return 1;
    }

    RequestMethod request_method;
    char *uri;
    HTTPVersion http_version;
    int process_request_line_result =
        ProcessRequestLine(context, connection, request_line, &request_method,
                           &uri, &http_version);
    if (process_request_line_result != 0) {
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
            ErrorResponse(context, connection, kHTTPVersionNotSupported);
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
            ErrorResponse(context, connection, kInternalServerError);
            free(uri);
            return 1;
        }
        if (field_line_length == 0) {
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
            if (strcmp(host_line, context->host) != 0 ||
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
            if (path[strlen(path) - 1] == '/') {
                char *concatenated = Format("%sindex.html", path);
                free(path);
                path = concatenated;
            }
            int serve_result = ServeFile(context, connection, path);
            free(path);
            return serve_result;
        }
        default: {
            ErrorResponse(context, connection, kBadRequest);
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
        ErrorResponse(context, connection, kInternalServerError);
        return 1;
    }
    if (request_method_string_length == 0) {
        ErrorResponse(context, connection, kBadRequest);
        return 1;
    }
    *request_method = GetRequestMethod(request_method_string);
    free(request_method_string);
    if (*request_method == 0) {
        ErrorResponse(context, connection, kBadRequest);
        return 1;
    }

    if (!isspace(request_line[request_method_string_length])) {
        ErrorResponse(context, connection, kBadRequest);
        return 1;
    }

    size_t uri_length;
    *uri = GetNextToken(request_line + request_method_string_length + 1,
                        &uri_length);
    if (uri == NULL) {
        Warning("failed to process URI: %s", strerror(errno));
        ErrorResponse(context, connection, kInternalServerError);
        return 1;
    }
    if (uri_length == 0) {
        ErrorResponse(context, connection, kBadRequest);
        free(*uri);
        return 1;
    }

    if (!isspace(request_line[request_method_string_length + 1 + uri_length])) {
        ErrorResponse(context, connection, kBadRequest);
        free(*uri);
        return 1;
    }

    size_t http_version_string_length;
    char *http_version_string = GetNextToken(
        request_line + request_method_string_length + 1 + uri_length + 1,
        &http_version_string_length);
    if (uri == NULL) {
        Warning("failed to process HTTP version: %s", strerror(errno));
        ErrorResponse(context, connection, kInternalServerError);
        free(*uri);
        return 1;
    }
    if (http_version_string_length == 0) {
        ErrorResponse(context, connection, kBadRequest);
        free(*uri);
        return 1;
    }
    *http_version = GetHTTPVersion(http_version_string);
    free(http_version_string);
    if (*http_version == 0) {
        ErrorResponse(context, connection, kBadRequest);
        free(*uri);
        return 1;
    }

    if (request_line[request_method_string_length + 1 + uri_length + 1 +
                     http_version_string_length] != '\0') {
        ErrorResponse(context, connection, kBadRequest);
        free(*uri);
        return 1;
    }

    return 0;
}

static size_t GetCommonHeader(const Context *context, char *buffer,
                              size_t buffer_size) {
    size_t n = 0;
    if (n < buffer_size) {
        n += GetDateHeader(buffer + n, buffer_size - n);
    }
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n, "Server: %s\r\n",
                      context->server);
    }
    return n;
}

static int ServeFile(const Context *context, int connection, const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ErrorResponse(context, connection, kNotFound);
        return 1;
    }
    SuccessResponse(context, connection, kOK);

    char buffer[BUFFER_SIZE];

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    snprintf(buffer, sizeof buffer, "Content-Length: %ld\r\n", file_size);
    send(connection, buffer, strlen(buffer), 0);

    const char *file_extension = strrchr(path, '.');
    const char *content_type = GetContentType(file_extension);
    if (content_type != NULL) {
        snprintf(buffer, sizeof buffer, "Content-Type: %s\r\n", content_type);
        send(connection, buffer, strlen(buffer), 0);
    }

    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);

    int send_file_error = SendFile(connection, file);
    if (send_file_error != 0) {
        Warning("failed to send response: %s", strerror(errno));
        return send_file_error;
    }
    return 0;
}

static void SuccessResponse(const Context *context, int connection,
                            ResponseStatusCode code) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n",
             GetHTTPVersionString(kHTTP_1_1), GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
    GetCommonHeader(context, buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
}

static void ErrorResponse(const Context *context, int connection,
                          ResponseStatusCode code) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n",
             GetHTTPVersionString(kHTTP_1_1), GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
    GetCommonHeader(context, buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "%s\n", GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
}
