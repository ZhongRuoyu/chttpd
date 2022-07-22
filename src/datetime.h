#ifndef CHTTPD_DATETIME_H_
#define CHTTPD_DATETIME_H_

#include <stddef.h>

#define DATE_HEADER_BUFFER_SIZE 36

int GetDateHeader(char *buffer, size_t buffer_size);

#endif  // CHTTPD_DATETIME_H_
