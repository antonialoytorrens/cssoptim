#include "args.h"
#include "argparse.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const usages[] = {
    "cssoptim [options] [[--] args]",
    "cssoptim [options]",
    NULL,
};

/* Argument parsing callbacks for the argparse library */

/* Callback for --css flag. Collects all subsequent non-hyphenated arguments as CSS files. */
static int css_cb(struct argparse *self, const struct argparse_option *opt) {
    css_args_t *args = (css_args_t *)opt->data;
    while (self->argc > 1 && self->argv[1][0] != '-') {
        if (args->css_file_count < MAX_INPUT_FILES) {
            args->css_files[args->css_file_count++] = self->argv[1];
        }
        self->argc--;
        self->argv++;
    }
    return 0;
}

static int html_cb(struct argparse *self, const struct argparse_option *opt) {
    css_args_t *args = (css_args_t *)opt->data;
    while (self->argc > 1 && self->argv[1][0] != '-') {
        if (args->html_file_count < MAX_INPUT_FILES) {
            args->html_files[args->html_file_count++] = self->argv[1];
        }
        self->argc--;
        self->argv++;
    }
    return 0;
}

int parse_args(int argc, const char **argv, css_args_t *args) {
    int result = 0;
    
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('v', "verbose", &args->verbose, "show verbose output", NULL, 0, 0),
        OPT_STRING('o', "output", &args->output_file, "output file path", NULL, 0, 0),
        OPT_STRING('r', "reduction", &args->reduction, "reduction mode: strict, safe, conservative (default: safe)", NULL, 0, 0),
        OPT_BOOLEAN(0, "css", NULL, "list of CSS files", css_cb, (intptr_t)args, 0),
        OPT_BOOLEAN(0, "html", NULL, "list of HTML/JS files", html_cb, (intptr_t)args, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nCSS Optimizer", "\nRemoves unused CSS classes.");
    
    argc = argparse_parse(&argparse, argc, argv);
    
    // Any remaining positional arguments can still be handled if needed,
    // but the user wants explicit flags.
    // For backward compatibility or extra files:
    /*
    if (argc > 0) {
        // ...
    }
    */
    
    return result;
}
