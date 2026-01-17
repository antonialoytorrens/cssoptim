#pragma once
#include <stdbool.h>

// Max input files for now, or use dynamic array
#define MAX_INPUT_FILES 64

typedef struct {
    const char *output_file;
    const char *css_files[MAX_INPUT_FILES];
    int css_file_count;
    const char *html_files[MAX_INPUT_FILES];
    int html_file_count;
    const char *reduction;
    bool verbose;
} css_args_t;

// Returns 0 on success, non-zero on error/help
int parse_args(int argc, const char **argv, css_args_t *args);
