#include "chttpd.h"

#include <ctype.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "datetime.h"
#include "http.h"
#include "logging.h"
#include "socket.h"
#include "strings.h"

static size_t GetCommonHeader(char *buffer, size_t buffer_size);
static int ServeFile(int connection, const char *path);
static void SuccessResponse(int connection, ResponseStatusCode code);
static void ErrorResponse(int connection, ResponseStatusCode code);

int ServeRequest(const char *host, const char *port, const char *root,
                 int connection, const char *from_addr_ip,
                 in_port_t from_addr_port) {
    char request_line[BUFFER_SIZE];
    size_t request_line_length =
        GetLineFromConnection(connection, request_line, sizeof request_line);

    char method[TOKEN_BUFFER_SIZE];
    char uri[URI_BUFFER_SIZE];
    char http_version_string[TOKEN_BUFFER_SIZE];
    {
        size_t p_request_line = 0;

        size_t method_length =
            GetNextToken(request_line + p_request_line, method, sizeof method);
        p_request_line += method_length;
        if (method_length == 0) {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }

        if (isspace(request_line[p_request_line])) {
            ++p_request_line;
        } else {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }

        size_t uri_length =
            GetNextToken(request_line + p_request_line, uri, sizeof uri);
        p_request_line += uri_length;
        if (uri_length == 0) {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }

        if (isspace(request_line[p_request_line])) {
            ++p_request_line;
        } else {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }

        size_t http_version_length =
            GetNextToken(request_line + p_request_line, http_version_string,
                         sizeof http_version_string);
        p_request_line += http_version_length;
        if (http_version_length == 0) {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }

        if (strlen(request_line + p_request_line) > 0) {
            ErrorResponse(connection, kBadRequest);
            return 1;
        }
    }

    HTTPVersion http_version = GetHTTPVersion(http_version_string);
    if (http_version == 0) {
        ErrorResponse(connection, kBadRequest);
        return 1;
    }
    switch (http_version) {
        case kHTTP_1_0:
        case kHTTP_1_1:
            break;
        default:
            ErrorResponse(connection, kHTTPVersionNotSupported);
            return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;
    char uri_host[LINE_BUFFER_SIZE] = "";
    char uri_port[TOKEN_BUFFER_SIZE] = "";
    while ((bytes_read =
                GetLineFromConnection(connection, buffer, sizeof buffer)) > 0) {
        if (strncasecmp("Host:", buffer, strlen("Host:")) == 0) {
            if (strnlen(uri_host, sizeof uri_host) > 0) {
                ErrorResponse(connection, kBadRequest);
                return 1;
            }
            TrimString(uri_host, buffer + strlen("Host:"), sizeof uri_host);
            char *port_seperator = strchr(uri_host, ':');
            if (port_seperator != NULL) {
                *port_seperator = '\0';
                CopyString(uri_port, port_seperator + 1, sizeof uri_port);
            }
        }
        // TODO: read and process request headers
    }
    if (http_version == kHTTP_1_1) {
        if (strnlen(uri_host, sizeof uri_host) == 0) {
            ErrorResponse(connection, kBadRequest);
            return 1;
        } else {
            if (strncmp(uri_host, host, sizeof uri_host) != 0 ||
                strnlen(uri_port, sizeof uri_port) > 0 &&
                    strncmp(uri_port, port, sizeof uri_port) != 0) {
                return 1;
            }
        }
    }

    RequestMethod request_method = GetRequestMethod(method);
    if (request_method == 0) {
        ErrorResponse(connection, kBadRequest);
        return 1;
    }

    LogRequestLine(from_addr_ip, from_addr_port, request_line);
    switch (request_method) {
        case kGET: {
            {
                char *query_string = strchr(uri, '?');
                if (query_string != NULL) {
                    *query_string = '\0';
                }
            }
            char path[BUFFER_SIZE];
            size_t path_length = 0;
            {
                size_t root_length = strlen(root);
                if (root_length + 1 >= sizeof path) {
                    ErrorResponse(connection, kURITooLong);
                    return 1;
                }
                CopyString(path, root, sizeof path);
                CopyString(path + root_length, uri, sizeof path - root_length);
                path_length = strnlen(path, sizeof path);
                if (path_length == sizeof path) {
                    ErrorResponse(connection, kURITooLong);
                    return 1;
                }
                if (path[path_length - 1] == '/') {
                    if (path_length + strlen("index.html") + 1 > sizeof path) {
                        ErrorResponse(connection, kURITooLong);
                        return 1;
                    }
                    CopyString(path + path_length, "index.html",
                               sizeof path - path_length);
                    path_length += strlen("index.html");
                }
            }
            return ServeFile(connection, path);
        }
        default: {
            // TODO: other request methods
            ErrorResponse(connection, kNotImplemented);
            return 1;
        }
    }

    ErrorResponse(connection, kNotImplemented);
    return 1;
}

static size_t GetCommonHeader(char *buffer, size_t buffer_size) {
    size_t n = 0;
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n, "Server: chttpd\r\n");
    }
    if (n < buffer_size) {
        n += GetDateHeader(buffer + n, buffer_size - n);
    }
    return n;
}

static int ServeFile(int connection, const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ErrorResponse(connection, kNotFound);
        return 1;
    }
    SuccessResponse(connection, kOK);

    char buffer[BUFFER_SIZE];

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    snprintf(buffer, sizeof buffer, "Content-Length: %ld\r\n", file_size);
    send(connection, buffer, strlen(buffer), 0);

    const char *file_extension = strrchr(path, '.');
    if (file_extension != NULL) {
        const char *content_type = NULL;
        if (strcasecmp(file_extension, ".css") == 0) {
            content_type = "text/css";
        } else if (strcasecmp(file_extension, ".html") == 0) {
            content_type = "text/html";
        } else if (strcasecmp(file_extension, ".js") == 0) {
            content_type = "text/javascript";
        }
        if (content_type != NULL) {
            snprintf(buffer, sizeof buffer, "Content-Type: %s\r\n",
                     content_type);
            send(connection, buffer, strlen(buffer), 0);
        }
    }

    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);

    int send_file_error = SendFile(connection, file);
    if (send_file_error != 0) {
        perror("failed to send response");
        return send_file_error;
    }
    return 0;
}

static void SuccessResponse(int connection, ResponseStatusCode code) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n",
             GetHTTPVersionString(kHTTP_1_1), GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
    GetCommonHeader(buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
}

static void ErrorResponse(int connection, ResponseStatusCode code) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n",
             GetHTTPVersionString(kHTTP_1_1), GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
    GetCommonHeader(buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "%s\n", GetResponseStatusString(code));
    send(connection, buffer, strlen(buffer), 0);
}
