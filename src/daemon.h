#ifndef CHTTPD_DAEMON_H_
#define CHTTPD_DAEMON_H_

#include "chttpd.h"

int InstallSignalHandlers(void);

int Daemon(Context *context);

#endif  // CHTTPD_DAEMON_H_
