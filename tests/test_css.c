#include "unity.h"
#include "../src/css_proc.h"
#include <string.h>
#include <stdlib.h>

void test_css_validation_basic(void) {
    const char *css = ".class { color: red; }";
    TEST_ASSERT_TRUE(css_validate(css, strlen(css)));
}

void test_css_validate_invalid(void) {
    // Liblexbor is error constant, so it might consume garbage, 
    // but let's see if we can detect empty or null
    TEST_ASSERT_FALSE(css_validate(NULL, 0));
}

void test_css_optimize_no_filter(void) {
    // If no used classes provided, it might return empty or null?
    // Or maybe we treat empty list as "remove everything"
    const char *css = ".foo { color: red; } .bar { color: blue; }";
    const char *used[] = {"foo"};
    
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 1,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_SAFE,
        .remove_unused_keyframes = true,
        .remove_form_pseudoelements = false
    };
    
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    // Should contain foo but not bar
    TEST_ASSERT_NOT_NULL(strstr(result, ".foo"));
    TEST_ASSERT_NULL(strstr(result, ".bar"));
    
    free(result);
}

void run_css_tests(void) {
    RUN_TEST(test_css_validation_basic);
    RUN_TEST(test_css_validate_invalid);
    RUN_TEST(test_css_optimize_no_filter);
}
