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
#include "context.h"
#include "errors.h"
#include "socket.h"

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

static int Initialize(const char *port) {
    struct addrinfo hints = {.ai_flags = AI_PASSIVE,
                             .ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *addr_info_head;

    {
        int gai_status = getaddrinfo(NULL, port, &hints, &addr_info_head);
        if (gai_status != 0) {
            Fatal("failed to get info for port %s: %s", port,
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
    Context *context = GetContext();

    ParseArguments(argc - 1, argv + 1, context);
    if (context->help) {
        Usage(stdout);
        exit(EXIT_SUCCESS);
    }

    int s = Initialize(context->port);
    printf("Listening at port %s...\n", context->port);

    for (;;) {
        struct sockaddr_storage from_addr_storage;
        socklen_t from_addr_storage_len = sizeof from_addr_storage;
        int connection = accept(s, (struct sockaddr *)&from_addr_storage,
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
            close(s);
            ServeRequest(context, connection, from_addr);
            close(connection);
            exit(EXIT_SUCCESS);
        } else {
            close(connection);
        }
    }

    close(s);

    exit(EXIT_SUCCESS);
}
