#ifndef CSSOPTIM_IO_H
#define CSSOPTIM_IO_H

#include <stdbool.h>
#include <stddef.h>

char *read_file(const char *filename, size_t *length);
bool write_file(const char *filename, const char *content);

#endif // CSSOPTIM_IO_H
