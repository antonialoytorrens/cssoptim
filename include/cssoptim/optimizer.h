#ifndef CSSOPTIM_OPTIMIZER_H
#define CSSOPTIM_OPTIMIZER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  LXB_CSS_OPTIM_MODE_STRICT,
  LXB_CSS_OPTIM_MODE_SAFE,
  LXB_CSS_OPTIM_MODE_CONSERVATIVE
} css_optim_mode_t;

typedef struct {
  const char **used_classes;
  size_t class_count;
  const char **used_tags;
  size_t tag_count;
  const char **used_attrs;
  size_t attr_count;

  bool remove_unused_keyframes;
  bool remove_form_pseudoelements;
  bool remove_vendor_prefixes;

  css_optim_mode_t mode;
} OptimizerConfig;

bool css_validate(const char *css_content, size_t length);
char *css_optimize(const char *css_content, size_t length,
                   OptimizerConfig *config);

#endif // CSSOPTIM_OPTIMIZER_H
