#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "chttpd.h"
#include "cmdline.h"
#include "daemon.h"
#include "log.h"
#include "socket.h"

static void BuildContext(Context *context, const Args *args) {
    context->daemon = args->daemon;
    if (args->host != NULL) {
        context->host = args->host;
    }
    if (args->index != NULL) {
        context->index = args->index;
    }
    if (args->log != NULL) {
        FILE *log_file = fopen(args->log, "w");
        if (log_file == NULL) {
            Fatal(context, "failed to open log file %s: %s", args->log,
                  strerror(errno));
        }
        context->log = log_file;
    }
    if (args->port != NULL) {
        context->port = args->port;
    }
    if (args->root != NULL) {
        context->root = args->root;
    }
    if (args->server != NULL) {
        context->server = args->server;
    }
}

static int Initialize(Context *context) {
    struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                             .ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *addr_info_head;

    {
        int gai_status =
            getaddrinfo(NULL, context->port, &hints, &addr_info_head);
        if (gai_status != 0) {
            Fatal(context, "failed to get info for port %s: %s", context->port,
                  gai_strerror(gai_status));
        }
    }

    int s = -1;
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
                freeaddrinfo(addr_info_head);
                close(s);
                Fatal(context, "failed to configure socket: %s",
                      strerror(errno));
            }
        }
        if (bind(s, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
            close(s);
            continue;
        }
        break;
    }
    if (addr_info == NULL) {
        freeaddrinfo(addr_info_head);
        Fatal(context, "failed to bind socket: %s", strerror(errno));
    }
    freeaddrinfo(addr_info_head);

    if (context->daemon) {
        if (Daemon(context) != 0) {
            if (context->log != NULL) {
                fclose(context->log);
                context->log = NULL;
            }
            close(s);
            Fatal(context, "failed to initialize daemon: %s", strerror(errno));
        }
    } else {
        if (InstallSignalHandlers() != 0) {
            close(s);
            Fatal(context, "failed to install signal handlers: %s",
                  strerror(errno));
        }
    }

    if (listen(s, BACKLOG) == -1) {
        close(s);
        Fatal(context, "failed to listen to socket: %s", strerror(errno));
    }

    return s;
}

int main(int argc, char **argv) {
    Args args = {
        .help = false,
        .version = false,
        .daemon = false,
        .host = NULL,
        .index = NULL,
        .log = NULL,
        .port = NULL,
        .root = NULL,
        .server = NULL,
    };
    ParseArgs(argc - 1, argv + 1, &args);
    if (args.help) {
        Usage(stdout);
        exit(EXIT_SUCCESS);
    }
    if (args.version) {
        Version(stdout);
        exit(EXIT_SUCCESS);
    }

    Context context = {
        .daemon = false,
        .host = NULL,
        .index = "index.html",
        .log = NULL,
        .port = "80",
        .root = ".",
        .server = "chttpd",
    };
    BuildContext(&context, &args);
    int socket = Initialize(&context);
    Info(&context, "listening at port %s", context.port);

    for (;;) {
        struct sockaddr_storage from_addr_storage;
        socklen_t from_addr_storage_len = sizeof from_addr_storage;
        int connection = accept(socket, (struct sockaddr *)&from_addr_storage,
                                &from_addr_storage_len);
        if (connection == -1) {
            Warning(&context, "failed to accept connection: %s",
                    strerror(errno));
            continue;
        }
        SocketAddress from_addr = GetSocketAddress(&from_addr_storage);

        pid_t child_pid = fork();
        if (child_pid == -1) {
            Warning(&context, "failed to create child process: %s",
                    strerror(errno));
            close(connection);
            continue;
        }
        if (child_pid == 0) {
            close(socket);
            ServeRequest(&context, connection, &from_addr);
            close(connection);
            exit(EXIT_SUCCESS);
        } else {
            close(connection);
        }
    }

    close(socket);
    if (context.log != NULL) {
        fclose(context.log);
        context.log = NULL;
    }
    exit(EXIT_SUCCESS);
}
