#ifndef CHTTPD_FILES_H_
#define CHTTPD_FILES_H_

#include <stdio.h>

const char *GetContentType(const char *file_extension);

int SendFile(int connection, FILE *file);

#endif  // CHTTPD_FILES_H_
