#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <lexbor/css/css.h>
#include <lexbor/css/parser.h>
#include <lexbor/css/stylesheet.h>
#include <lexbor/css/rule.h>
#include <lexbor/core/serialize.h>
#include <lexbor/css/at_rule.h>

// Simple serializer callback
lxb_status_t serializer_callback(const lxb_char_t *data, size_t len, void *ctx) {
    char **buffer = (char **)ctx;
    size_t current_len = *buffer ? strlen(*buffer) : 0;
    char *new_buf = realloc(*buffer, current_len + len + 1);
    if (!new_buf) return LXB_STATUS_ERROR;
    *buffer = new_buf;
    memcpy(*buffer + current_len, data, len);
    (*buffer)[current_len + len] = '\0';
    return LXB_STATUS_OK;
}

// Prototype
char *css_optimize(const char *css, size_t css_len, const char **used_classes, size_t class_count);

int main(int argc, const char **argv) {
    const char *css = 
        "@media (min-width: 500px) {\n"
        "    .used { color: var(--used-var); animation: slide 1s; }\n"
        "    .unused { color: red; }\n"
        "}\n"
        ":root { --used-var: blue; --unused-var: red; }\n"
        "@keyframes slide { from { opacity: 0; } }\n"
        "@keyframes unused-anim { from { opacity: 0; } }";
        
    printf("Input CSS:\n%s\n\n", css);
    
    const char *used[] = {"used"};
    char *result = css_optimize(css, strlen(css), used, 1);
    
    if (result) {
        printf("Optimized CSS:\n%s\n", result);
        free(result);
    } else {
        printf("Optimization failed.\n");
    }
    
    return 0;
}
