#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdio.h>

int write_all(int fd, const char *data, size_t length);
ssize_t read_line_retry(FILE *stream, char **line, size_t *capacity);

#endif