#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "chttpd.h"
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

    if (InstallSignalHandler() != 0) {
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

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: httpd <host> <port> <path-to-root>\n");
        exit(EXIT_FAILURE);
    }
    const char *host = argv[1];
    const char *port = argv[2];
    const char *root = argv[3];
    int s = Initialize(port);
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
            ServeRequest(host, port, root, connection, from_addr_ip,
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
