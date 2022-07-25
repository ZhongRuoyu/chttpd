#include "daemon.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "log.h"

static void SignalHandler(int signo, siginfo_t *info, void *ucontext) {
    switch (signo) {
        case SIGHUP: {
            break;
        }
        case SIGINT: {
            Info(NULL, "SIGINT received; shutting down");
            while (waitpid(-1, NULL, 0) > 0) {
                continue;
            }
            Info(NULL, "shutdown completed");
            exit(EXIT_SUCCESS);
        }
        case SIGTERM: {
            Info(NULL, "SIGTERM received; shutting down");
            while (waitpid(-1, NULL, 0) > 0) {
                continue;
            }
            Info(NULL, "shutdown completed");
            exit(EXIT_SUCCESS);
        }
        case SIGCHLD: {
            int saved_errno = errno;
            while (waitpid(-1, NULL, WNOHANG) > 0) {
                continue;
            }
            errno = saved_errno;
            break;
        }
    }
}

int InstallSignalHandlers() {
    struct sigaction action = {.sa_sigaction = SignalHandler,
                               .sa_flags = SA_RESTART | SA_SIGINFO};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGHUP, &action, NULL) != 0) {
        return -1;
    }
    if (sigaction(SIGINT, &action, NULL) != 0) {
        return -1;
    }
    if (sigaction(SIGTERM, &action, NULL) != 0) {
        return -1;
    }
    if (sigaction(SIGCHLD, &action, NULL) != 0) {
        return -1;
    }
    return 0;
}

static int ForkAndExit() {
    pid_t child_pid = fork();
    if (child_pid == -1) {
        return -1;
    }
    if (child_pid > 0) {
        exit(EXIT_SUCCESS);
    }
    return 0;
}

int Daemon(const Context *context) {
    if (ForkAndExit() != 0) {
        return -1;
    }
    if (setsid() == -1) {
        return -1;
    }
    if (ForkAndExit() != 0) {
        return -1;
    }
    if (InstallSignalHandlers() != 0) {
        return -1;
    }
    umask(0);
    int dev_null_fd = open("/dev/null", O_RDWR);
    if (dev_null_fd == -1) {
        return -1;
    }
    if (dup2(dev_null_fd, STDIN_FILENO) == -1) {
        return -1;
    }
    if (dup2(dev_null_fd, STDOUT_FILENO) == -1) {
        return -1;
    }
    if (context->log != NULL) {
        if (dup2(fileno(context->log), STDERR_FILENO) == -1) {
            return -1;
        }
    } else {
        if (dup2(dev_null_fd, STDERR_FILENO) == -1) {
            return -1;
        }
    }
    if (close(dev_null_fd) == -1) {
        return -1;
    }
    return 0;
}
