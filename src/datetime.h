#ifndef CHTTPD_DATETIME_H_
#define CHTTPD_DATETIME_H_

#include <stddef.h>

static const size_t kDateHeaderBufferSize = 36;

int GetDateHeader(char *buffer, size_t buffer_size);

static const size_t kLogDateBufferSize = 0;

int GetLogDate(char *buffer, size_t buffer_size);

#endif  // CHTTPD_DATETIME_H_
