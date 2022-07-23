#ifndef CHTTPD_CMDLINE_H_
#define CHTTPD_CMDLINE_H_

#include <stdio.h>

#include "context.h"

void Usage(FILE *out);

void ParseArguments(int argc, char **argv, Context *context);

#endif  // CHTTPD_CMDLINE_H_
