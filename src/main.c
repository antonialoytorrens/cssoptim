#include "args.h"
#include "css_proc.h"
#include "html_scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <physfs.h>

/* Main entry point for the CSS Optimizer.
 * Coordinates scanning of input files (HTML/JS) and optimization of CSS files.
 */

// Helper to read file content using PhysFS from the mounted virtual filesystem
static char *read_file(const char *filename, size_t *length) {
    if (!PHYSFS_exists(filename)) return NULL;

    PHYSFS_File *f = PHYSFS_openRead(filename);
    if (!f) return NULL;
    
    PHYSFS_sint64 size = PHYSFS_fileLength(f);
    if (size < 0) { PHYSFS_close(f); return NULL; }
    
    char *buf = malloc(size + 1);
    if (!buf) { PHYSFS_close(f); return NULL; }
    
    // PHYSFS_read returns number of objects read, which acts like bytes here
    if (PHYSFS_readBytes(f, buf, size) < size) {
        free(buf);
        PHYSFS_close(f);
        return NULL;
    }
    
    buf[size] = '\0';
    if (length) *length = (size_t)size;
    
    PHYSFS_close(f);
    return buf;
}

// Helper to write file content using PhysFS
bool write_file(const char *filename, const char *content) {
    PHYSFS_File *f = PHYSFS_openWrite(filename);
    if (!f) return false;
    
    size_t len = strlen(content);
    if (PHYSFS_writeBytes(f, content, len) < (PHYSFS_sint64)len) {
        PHYSFS_close(f);
        return false;
    }
    
    PHYSFS_close(f);
    return true;
}

// Helper to check file extension for determining scan mode
static bool has_extension(const char *filename, const char *ext) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return false;
    return strcmp(dot + 1, ext) == 0;
}

int main(int argc, const char **argv) {
    /* Step 1: Initialize PhysFS for virtual file I/O */
    // Initialize PhysFS
    if (!PHYSFS_init(argv[0])) {
        fprintf(stderr, "Error: Could not initialize PhysFS: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return 1;
    }

    // Mount current directory to support reading files relative to CWD
    // usage: ./cssoptim input.css -> reads ./input.css
    if (!PHYSFS_mount(".", NULL, 1)) {
        fprintf(stderr, "Error: Could not mount current directory: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        PHYSFS_deinit();
        return 1;
    }
    
    // Set write dir to CWD to support writing output
    if (!PHYSFS_setWriteDir(".")) {
         fprintf(stderr, "Error: Could not set write dir to current directory: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
         PHYSFS_deinit();
         return 1;
    }

    css_args_t args = {0};
    if (parse_args(argc, argv, &args) != 0) {
        PHYSFS_deinit();
        return 1;
    }
    
    // Scan HTML/JS files for classes and tags
    string_list_t used_classes;
    string_list_init(&used_classes);
    string_list_t used_tags;
    string_list_init(&used_tags);
    
    // Process HTML/JS files
    for (int i = 0; i < args.html_file_count; i++) {
        const char *fname = args.html_files[i];
        size_t len = 0;
        char *content = read_file(fname, &len);
        
        if (!content) {
            fprintf(stderr, "Warning: Could not read file %s (Reason: %s)\n", fname, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            continue;
        }
        
        if (has_extension(fname, "html") || has_extension(fname, "htm")) {
            if (args.verbose) printf("Scanning HTML: %s\n", fname);
            scan_html(content, len, &used_classes, &used_tags);
        } else if (has_extension(fname, "js") || has_extension(fname, "jsx") || has_extension(fname, "ts")) {
            if (args.verbose) printf("Scanning JS: %s\n", fname);
            scan_js(content, len, &used_classes);
        } else {
            // If explicitly passed under --html but unknown extension, assume HTML?
            // Or just scan anyway as HTML.
            if (args.verbose) printf("Scanning unknown file type as HTML: %s\n", fname);
            scan_html(content, len, &used_classes, &used_tags);
        }
        
        free(content);
    }
    
    if (args.verbose) {
        printf("Found %zu used classes:\n", used_classes.count);
        for (size_t i = 0; i < used_classes.count; i++) {
            printf("  - %s\n", used_classes.items[i]);
        }
        printf("Found %zu used tags:\n", used_tags.count);
        for (size_t i = 0; i < used_tags.count; i++) {
            printf("  - %s\n", used_tags.items[i]);
        }
    }
    
    // Process CSS files
    bool success = true;
    for (int i = 0; i < args.css_file_count; i++) {
        const char *fname = args.css_files[i];
        if (args.verbose) printf("Processing CSS: %s\n", fname);
        
        size_t len = 0;
        char *content = read_file(fname, &len);
        if (content) {
            // Pass both used classes and used tags to the optimizer
            char *optimized = css_optimize(content, len, 
                                          (const char **)used_classes.items, used_classes.count,
                                          (const char **)used_tags.items, used_tags.count);
            if (optimized) {
                if (args.output_file) {
                    // Write to file using PhysFS
                     if (!write_file(args.output_file, optimized)) {
        fprintf(stderr, "Error: Could not write output file %s: %s\n", args.output_file, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
                         success = false;
                     }
                } else {
                    // Write to stdout (PhysFS doesn't wrap stdout/stderr easily)
                    printf("%s\n", optimized);
                }
                free(optimized);
            } else {
                fprintf(stderr, "Error optimizing CSS file: %s\n", fname);
                success = false;
            }
            free(content);
        } else {
            fprintf(stderr, "Error: Could not read CSS file %s: %s\n", fname, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            success = false;
        }
    }
    
    string_list_destroy(&used_classes);
    string_list_destroy(&used_tags);
    PHYSFS_deinit();
    
    return success ? 0 : 1;
}
