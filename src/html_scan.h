#pragma once
#include <stddef.h>
#include <stdbool.h>

// Opaque or ZPL array of strings
// Opaque or ZPL array of strings
typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} string_list_t;

void string_list_init(string_list_t *list);
void string_list_destroy(string_list_t *list);
void string_list_add(string_list_t *list, const char *str);
bool string_list_contains(string_list_t *list, const char *str);

// Scan HTML content and add classes/tags to lists
// tags can be NULL if only classes are needed
void scan_html(const char *content, size_t length, string_list_t *classes, string_list_t *tags, string_list_t *attrs);

// Scan JS content and add potential classes to list
void scan_js(const char *content, size_t length, string_list_t *classes);
