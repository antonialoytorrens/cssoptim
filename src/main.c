#include "args.h"
#include "cssoptim/io.h"
#include "cssoptim/optimizer.h"
#include "cssoptim/scanner.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Main entry point for the CSS Optimizer.
 * Coordinates scanning of input files (HTML/JS) and optimization of CSS files.
 */



// Helper to check file extension for determining scan mode
static bool has_extension(const char *filename, const char *ext) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return false;
  return strcmp(dot + 1, ext) == 0;
}

int main(int argc, const char **argv) {
  css_args_t args = {0};
  if (parse_args(argc, argv, &args) != 0) {
    return 1;
  }

  // Scan HTML/JS files for classes and tags
  string_list_t *used_classes = string_list_create();
  string_list_t *used_tags = string_list_create();
  string_list_t *used_attrs = string_list_create();

  if (!used_classes || !used_tags || !used_attrs) {
    fprintf(stderr, "Error: Failed to initialize string lists\n");
    return 1;
  }

  // Process HTML/JS files
  for (int i = 0; i < args.html_file_count; i++) {
    const char *fname = args.html_files[i];
    size_t len = 0;
    char *content = read_file(fname, &len);

    if (!content) {
      fprintf(stderr, "Warning: Could not read file %s (Reason: %s)\n", fname,
              strerror(errno));
      continue;
    }

    if (has_extension(fname, "html") || has_extension(fname, "htm")) {
      if (args.verbose)
        printf("Scanning HTML: %s\n", fname);
      scan_html(content, len, used_classes, used_tags, used_attrs);
    } else if (has_extension(fname, "js") || has_extension(fname, "jsx") ||
               has_extension(fname, "ts")) {
      if (args.verbose)
        printf("Scanning JS: %s\n", fname);
      scan_js(content, len, used_classes);
    } else {
      if (args.verbose)
        printf("Scanning unknown file type as HTML: %s\n", fname);
      scan_html(content, len, used_classes, used_tags, used_attrs);
    }

    free(content);
  }

  if (args.verbose) {
    printf("Found %zu used classes:\n", string_list_count(used_classes));
    for (size_t i = 0; i < string_list_count(used_classes); i++) {
      printf("  - %s\n", string_list_get(used_classes, i));
    }
    printf("Found %zu used tags:\n", string_list_count(used_tags));
    for (size_t i = 0; i < string_list_count(used_tags); i++) {
      printf("  - %s\n", string_list_get(used_tags, i));
    }
    printf("Collected %zu unique attributes:\n", string_list_count(used_attrs));
    for (size_t i = 0; i < string_list_count(used_attrs); i++) {
      printf("  - %s\n", string_list_get(used_attrs, i));
    }
  }

  // Determine reduction mode
  css_optim_mode_t mode = LXB_CSS_OPTIM_MODE_SAFE; // Default
  if (args.reduction) {
    if (strcmp(args.reduction, "strict") == 0) {
      mode = LXB_CSS_OPTIM_MODE_STRICT;
    } else if (strcmp(args.reduction, "conservative") == 0) {
      mode = LXB_CSS_OPTIM_MODE_CONSERVATIVE;
    } else if (strcmp(args.reduction, "safe") == 0) {
      mode = LXB_CSS_OPTIM_MODE_SAFE;
    } else {
      fprintf(stderr, "Warning: Unknown reduction mode '%s'. Using 'safe'.\n",
              args.reduction);
    }
  }

  // Process CSS files
  bool success = true;
  for (int i = 0; i < args.css_file_count; i++) {
    const char *fname = args.css_files[i];
    if (args.verbose)
      printf("Processing CSS: %s\n", fname);

    size_t len = 0;
    char *content = read_file(fname, &len);
    if (content) {
      // Pass used classes, tags, and attributes to the optimizer
      OptimizerConfig config = {
          .used_classes = string_list_items(used_classes),
          .class_count = string_list_count(used_classes),
          .used_tags = string_list_items(used_tags),
          .tag_count = string_list_count(used_tags),
          .used_attrs = string_list_items(used_attrs),
          .attr_count = string_list_count(used_attrs),
          .mode = mode,
          .remove_unused_keyframes = true,
          .remove_form_pseudoelements =
              (string_list_count(used_tags) > 0) // Only if we have tag info
      };

      char *optimized = css_optimize(content, len, &config);
      if (optimized) {
        if (args.output_file) {
          if (!write_file(args.output_file, optimized)) {
            fprintf(stderr, "Error: Could not write output file %s: %s\n",
                    args.output_file, strerror(errno));
            success = false;
          }
        } else {
          printf("%s\n", optimized);
        }
        free(optimized);
      } else {
        fprintf(stderr, "Error optimizing CSS file: %s\n", fname);
        success = false;
      }
      free(content);
    } else {
      fprintf(stderr, "Error: Could not read CSS file %s: %s\n", fname,
              strerror(errno));
      success = false;
    }
  }

  string_list_destroy(used_classes);
  string_list_destroy(used_tags);
  string_list_destroy(used_attrs);

  return success ? 0 : 1;
}
