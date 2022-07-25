#ifndef CHTTPD_DAEMON_H_
#define CHTTPD_DAEMON_H_

#include "chttpd.h"

int InstallSignalHandlers();

int Daemon(const Context *context);

#endif  // CHTTPD_DAEMON_H_
