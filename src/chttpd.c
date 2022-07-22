#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "datetime.h"
#include "http.h"
#include "socket.h"
#include "strings.h"

#define BACKLOG SOMAXCONN

#define BUFFER_SIZE 4096
#define TOKEN_BUFFER_SIZE 256
#define LINE_BUFFER_SIZE 1024
#define URI_BUFFER_SIZE 2048

#define SERVER "chttpd"

#define INDEX "index.html"

#define HTTP_VERSION "HTTP/1.1"
#define HTTP_VERSION_MAJOR 1
#define HTTP_VERSION_MINOR 1

void sigchld_handler(int arg) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
    errno = saved_errno;
}

void get_common_header(char *buffer, size_t buffer_size) {
    int n = 0;
    if (n < buffer_size) {
        n += snprintf(buffer + n, buffer_size - n, "%s%s\r\n",
                      RESPONSE_HEADER_SERVER, SERVER);
    }
    if (n < buffer_size) {
        n += GetDateHeader(buffer + n, buffer_size - n);
    }
}

int initialize(const char *port);
int serve_request(const char *host, const char *port, const char *root,
                  int connection, const char *from_addr_ip,
                  in_port_t from_addr_port);
int serve_file(int connection, const char *path);
int send_file(int connection, FILE *file);
void success_response(int connection, const char *status);
void error_response(int connection, const char *status);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <host> <port> <path-to-root>\n", SERVER);
        exit(EXIT_FAILURE);
    }
    const char *host = argv[1];
    const char *port = argv[2];
    const char *root = argv[3];
    int s = initialize(port);
    printf("Listening at port %s...\n", port);

    for (;;) {
        struct sockaddr_storage from_addr;
        socklen_t from_addr_len = sizeof from_addr;
        int connection =
            accept(s, (struct sockaddr *)&from_addr, &from_addr_len);
        if (connection == -1) {
            perror("failed to accept connection");
            continue;
        }

        char from_addr_ip[INET6_ADDRSTRLEN];
        inet_ntop(from_addr.ss_family,
                  GetInAddr((const struct sockaddr *)&from_addr), from_addr_ip,
                  sizeof from_addr_ip);
        in_port_t from_addr_port =
            GetInPort((const struct sockaddr *)&from_addr);

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("failed to create child process");
            close(connection);
            continue;
        }
        if (child_pid == 0) {
            close(s);
            serve_request(host, port, root, connection, from_addr_ip,
                          from_addr_port);
            close(connection);
            exit(EXIT_SUCCESS);
        } else {
            close(connection);
        }
    }

    close(s);

    return 0;
}

int initialize(const char *port) {
    struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                             .ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *addr_info_head;

    {
        int gai_status = getaddrinfo(NULL, port, &hints, &addr_info_head);
        if (gai_status != 0) {
            fprintf(stderr, "failed to get port info: %s\n",
                    gai_strerror(gai_status));
            exit(EXIT_FAILURE);
        }
    }

    int s;
    struct addrinfo *addr_info;
    for (addr_info = addr_info_head; addr_info != NULL;
         addr_info = addr_info->ai_next) {
        s = socket(addr_info->ai_family, addr_info->ai_socktype,
                   addr_info->ai_protocol);
        if (s == -1) {
            continue;
        }
        {
            int yes = 1;
            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ==
                -1) {
                perror("failed to configure socket");
                freeaddrinfo(addr_info_head);
                close(s);
                exit(EXIT_FAILURE);
            }
        }
        if (bind(s, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
            close(s);
            continue;
        }
        break;
    }
    if (addr_info == NULL) {
        perror("failed to bind socket");
        freeaddrinfo(addr_info_head);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(addr_info_head);

    struct sigaction action = {.sa_handler = sigchld_handler,
                               .sa_flags = SA_RESTART};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGCHLD, &action, NULL) == -1) {
        perror("failed to set up signal handler");
        close(s);
        exit(EXIT_FAILURE);
    }

    if (listen(s, BACKLOG) == -1) {
        perror("failed to listen");
        close(s);
        exit(EXIT_FAILURE);
    }

    return s;
}

int serve_request(const char *host, const char *port, const char *root,
                  int connection, const char *from_addr_ip,
                  in_port_t from_addr_port) {
    char log_time[kDateHeaderBufferSize];
    GetLogDate(log_time, sizeof log_time);

    char request_line[BUFFER_SIZE];
    size_t request_line_length =
        GetLineFromConnection(connection, request_line, sizeof request_line);

    char method[TOKEN_BUFFER_SIZE];
    char uri[URI_BUFFER_SIZE];
    char http_version[TOKEN_BUFFER_SIZE];
    {
        size_t p_request_line = 0;

        size_t method_length =
            GetNextToken(request_line + p_request_line, method, sizeof method);
        p_request_line += method_length;
        if (method_length == 0) {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }

        if (isspace(request_line[p_request_line])) {
            ++p_request_line;
        } else {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }

        size_t uri_length =
            GetNextToken(request_line + p_request_line, uri, sizeof uri);
        p_request_line += uri_length;
        if (uri_length == 0) {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }

        if (isspace(request_line[p_request_line])) {
            ++p_request_line;
        } else {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }

        size_t http_version_length = GetNextToken(
            request_line + p_request_line, http_version, sizeof http_version);
        p_request_line += http_version_length;
        if (http_version_length == 0) {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }

        if (strlen(request_line + p_request_line) > 0) {
            error_response(connection, GetResponseStatus(kBadRequest));
            return 1;
        }
    }

    int http_version_major;
    int http_version_minor;
    if (GetHTTPVersion(http_version, &http_version_major,
                       &http_version_minor) != 0) {
        error_response(connection, GetResponseStatus(kBadRequest));
        return 1;
    }
    if (HTTP_VERSION_MAJOR < http_version_major ||
        (HTTP_VERSION_MAJOR == http_version_major &&
         HTTP_VERSION_MINOR < http_version_minor)) {
        error_response(connection, GetResponseStatus(kHTTPVersionNotSupported));
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;
    char uri_host[LINE_BUFFER_SIZE] = "";
    char uri_port[TOKEN_BUFFER_SIZE] = "";
    while ((bytes_read =
                GetLineFromConnection(connection, buffer, sizeof buffer)) > 0) {
        if (strncasecmp(REQUEST_HEADER_HOST, buffer,
                        strlen(REQUEST_HEADER_HOST)) == 0) {
            if (strnlen(uri_host, sizeof uri_host) > 0) {
                error_response(connection, GetResponseStatus(kBadRequest));
                return 1;
            }
            TrimString(uri_host, buffer + strlen(REQUEST_HEADER_HOST),
                       sizeof uri_host);
            char *port_seperator = strchr(uri_host, ':');
            if (port_seperator != NULL) {
                *port_seperator = '\0';
                CopyString(uri_port, port_seperator + 1, sizeof uri_port);
            }
        }
        // TODO: read and process request headers
    }
    if (http_version_major > 1 ||
        http_version_major == 1 && http_version_minor >= 1) {
        if (strnlen(uri_host, sizeof uri_host) == 0) {
            error_response(connection, GetResponseStatus(kBadRequest));
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
        error_response(connection, GetResponseStatus(kBadRequest));
        return 1;
    }

    printf("[%s [%s]:%d] %s\n", log_time, from_addr_ip, from_addr_port,
           request_line);
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
                    error_response(connection, GetResponseStatus(kURITooLong));
                    return 1;
                }
                CopyString(path, root, sizeof path);
                CopyString(path + root_length, uri, sizeof path - root_length);
                path_length = strnlen(path, sizeof path);
                if (path_length == sizeof path) {
                    error_response(connection, GetResponseStatus(kURITooLong));
                    return 1;
                }
                if (path[path_length - 1] == '/') {
                    if (path_length + strlen(INDEX) + 1 > sizeof path) {
                        error_response(connection,
                                       GetResponseStatus(kURITooLong));
                        return 1;
                    }
                    CopyString(path + path_length, INDEX,
                               sizeof path - path_length);
                    path_length += strlen(INDEX);
                }
            }
            return serve_file(connection, path);
        }
        default: {
            // TODO: other request methods
            error_response(connection, GetResponseStatus(kNotImplemented));
            return 1;
        }
    }

    error_response(connection, GetResponseStatus(kNotImplemented));
    return 1;
}

int serve_file(int connection, const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        error_response(connection, GetResponseStatus(kNotFound));
        return 1;
    }
    success_response(connection, GetResponseStatus(kOK));

    char buffer[BUFFER_SIZE];

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    snprintf(buffer, sizeof buffer, "%s%ld\r\n", RESPONSE_HEADER_CONTENT_LENGTH,
             file_size);
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
            snprintf(buffer, sizeof buffer, "%s%s\r\n",
                     RESPONSE_HEADER_CONTENT_TYPE, content_type);
            send(connection, buffer, strlen(buffer), 0);
        }
    }

    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);

    return send_file(connection, file);
}

int send_file(int connection, FILE *file) {
    char buffer[BUFFER_SIZE];
    for (;;) {
        size_t bytes_read = fread(buffer, sizeof(char), sizeof buffer, file);
        if (bytes_read == 0) {
            break;
        }
        ssize_t bytes_sent = send(connection, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            perror("failed to send response");
            return 1;
        }
    }
    return 0;
}

void success_response(int connection, const char *status) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n", HTTP_VERSION, status);
    send(connection, buffer, strlen(buffer), 0);
    get_common_header(buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
}

void error_response(int connection, const char *status) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof buffer, "%s %s\r\n", HTTP_VERSION, status);
    send(connection, buffer, strlen(buffer), 0);
    get_common_header(buffer, sizeof buffer);
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "\r\n");
    send(connection, buffer, strlen(buffer), 0);
    snprintf(buffer, sizeof buffer, "%s\n", status);
    send(connection, buffer, strlen(buffer), 0);
}
