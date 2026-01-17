#include "unity.h"
#include "../src/css_proc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

int main() {
    UNITY_BEGIN();
    
    const char *css = "div::before { content: ''; } span::after { color: blue; }";
    const char *used_tags[] = {"div"};
    
    printf("Input CSS: %s\n", css);
    printf("Used tags: div\n");
    
    char *result = css_optimize(css, strlen(css), NULL, 0, used_tags, 1, NULL, 0, LXB_CSS_OPTIM_MODE_STRICT);
    
    printf("Output CSS: %s\n", result ? result : "NULL");
    
    if (result) {
        if (strstr(result, "span::after")) {
            printf("ERROR: span::after was NOT removed!\n");
        } else {
            printf("SUCCESS: span::after was removed\n");
        }
        
        if (strstr(result, "div::before")) {
            printf("SUCCESS: div::before was kept\n");
        } else {
            printf("ERROR: div::before was removed!\n");
        }
        
        free(result);
    }
    
    return UNITY_END();
}
