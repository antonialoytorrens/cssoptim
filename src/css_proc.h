#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char *raw_css;
    // Opaque pointer to Lexbor CSS object if needed, 
    // but for now let's just expose a function
} css_context_t;

typedef enum {
    LXB_CSS_OPTIM_MODE_STRICT,       // Only used classes/tags/attributes
    LXB_CSS_OPTIM_MODE_SAFE,         // STRICT + universal resets (*) + used variables
    LXB_CSS_OPTIM_MODE_CONSERVATIVE  // SAFE + browser-prefixed pseudos + accessibility media queries
} css_optim_mode_t;

typedef struct {
    const char **used_classes;
    size_t class_count;
    const char **used_tags;
    size_t tag_count;
    const char **used_attrs;
    size_t attr_count;
    
    // Options
    bool remove_unused_keyframes;      // true (recomanat)
    bool remove_form_pseudoelements;   // true si no hi ha forms
    bool remove_vendor_prefixes;       // false (mantenir compatibilitat)
    
    css_optim_mode_t mode;
} OptimizerConfig;

// Parses CSS and returns a handle (or just validates for now)
bool css_validate(const char *css_content, size_t length);

// Minify/Optimize CSS by removing unused classes and tags
// Returns new allocated string (must be freed) or NULL on error
char *css_optimize(const char *css_content, size_t length, OptimizerConfig *config);
