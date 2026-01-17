#ifndef CSSOPTIM_SCANNER_H
#define CSSOPTIM_SCANNER_H

#include "list.h"
#include <stddef.h>

void scan_html(const char *content, size_t length, string_list_t *classes,
               string_list_t *tags, string_list_t *attrs);
void scan_js(const char *content, size_t length, string_list_t *classes);

#endif // CSSOPTIM_SCANNER_H
