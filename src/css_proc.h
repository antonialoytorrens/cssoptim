#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char *raw_css;
    // Opaque pointer to Lexbor CSS object if needed, 
    // but for now let's just expose a function
} css_context_t;

// Parses CSS and returns a handle (or just validates for now)
bool css_validate(const char *css_content, size_t length);

// Minify/Optimize CSS by removing unused classes and tags
// used_classes: array of class names to KEEP
// used_tags: array of tag names to KEEP (e.g. "div", "h1")
// Returns new allocated string (must be freed) or NULL on error
char *css_optimize(const char *css_content, size_t length, 
                   const char **used_classes, size_t class_count,
                   const char **used_tags, size_t tag_count);
