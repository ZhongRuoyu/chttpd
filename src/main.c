#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "chttpd.h"
#include "cmdline.h"
#include "log.h"
#include "socket.h"

static void BuildContext(Context *context, const Args *args) {
    if (args->host != NULL) {
        context->host = args->host;
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

static void SigchldHandler(int arg) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
    errno = saved_errno;
}

static int InstallSignalHandler() {
    struct sigaction action = {.sa_handler = SigchldHandler,
                               .sa_flags = SA_RESTART};
    sigemptyset(&action.sa_mask);

    int error;
    error = sigaction(SIGCHLD, &action, NULL);
    if (error != 0) {
        return -1;
    }

    return 0;
}

static int Initialize(const Context *context) {
    struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                             .ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *addr_info_head;

    {
        int gai_status =
            getaddrinfo(NULL, context->port, &hints, &addr_info_head);
        if (gai_status != 0) {
            Fatal("failed to get info for port %s: %s", context->port,
                  gai_strerror(gai_status));
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
                freeaddrinfo(addr_info_head);
                close(s);
                Fatal("failed to configure socket: %s", strerror(errno));
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
        Fatal("failed to bind socket: %s", strerror(errno));
    }
    freeaddrinfo(addr_info_head);

    if (InstallSignalHandler() != 0) {
        close(s);
        Fatal("failed to set up signal handler: %s", strerror(errno));
    }

    if (listen(s, BACKLOG) == -1) {
        close(s);
        Fatal("failed to listen to socket: %s", strerror(errno));
    }

    return s;
}

int main(int argc, char **argv) {
    Args args = {
        .help = false,
        .version = false,
        .host = NULL,
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
        .host = NULL,
        .port = "80",
        .root = ".",
        .server = "chttpd",
    };
    BuildContext(&context, &args);
    int socket = Initialize(&context);
    Info("listening at port %s", context.port);

    for (;;) {
        struct sockaddr_storage from_addr_storage;
        socklen_t from_addr_storage_len = sizeof from_addr_storage;
        int connection = accept(socket, (struct sockaddr *)&from_addr_storage,
                                &from_addr_storage_len);
        if (connection == -1) {
            Warning("failed to accept connection: %s", strerror(errno));
            continue;
        }
        SocketAddress from_addr = GetSocketAddress(&from_addr_storage);

        pid_t child_pid = fork();
        if (child_pid == -1) {
            Warning("failed to create child process: %s", strerror(errno));
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

    exit(EXIT_SUCCESS);
}
