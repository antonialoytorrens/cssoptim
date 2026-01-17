#include "unity.h"
#include "../src/css_proc.h"
#include <string.h>
#include <stdlib.h>

void test_remove_unused_keyframes(void) {
    const char *css = "@keyframes used { from { opacity: 0; } } @keyframes unused { from { opacity: 1; } } .class { animation: used 1s; }";
    const char *used_classes[] = {"class"};
    
    OptimizerConfig config = {
        .used_classes = used_classes,
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
    // Should contain 'used' but not 'unused'
    TEST_ASSERT_NOT_NULL(strstr(result, "@keyframes used"));
    TEST_ASSERT_NULL(strstr(result, "@keyframes unused"));
    
    free(result);
}

void test_remove_form_pseudoelements_without_forms(void) {
    const char *css = ".test { color: red } ::file-selector-button { font: inherit } ::-webkit-inner-spin-button { appearance: none }";
    const char *used_classes[] = {"test"};
    const char *used_tags[] = {"div"};
    // No type="file" or type="number" in attributes
    
    OptimizerConfig config = {
        .used_classes = used_classes,
        .class_count = 1,
        .used_tags = used_tags,
        .tag_count = 1,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_SAFE,
        .remove_unused_keyframes = true,
        .remove_form_pseudoelements = true
    };
    
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    
    TEST_ASSERT_NOT_NULL(strstr(result, ".test"));
    TEST_ASSERT_NULL(strstr(result, "::file-selector-button"));
    TEST_ASSERT_NULL(strstr(result, "-webkit-inner-spin-button"));
    
    free(result);
}

void test_keep_form_pseudoelements_with_forms(void) {
    const char *css = "::file-selector-button { font: inherit }";
    const char *used_tags[] = {"input"};
    const char *used_attrs[] = {"type=file"};
    
    OptimizerConfig config = {
        .used_classes = NULL,
        .class_count = 0,
        .used_tags = used_tags,
        .tag_count = 1,
        .used_attrs = used_attrs,
        .attr_count = 1,
        .mode = LXB_CSS_OPTIM_MODE_SAFE,
        .remove_unused_keyframes = true,
        .remove_form_pseudoelements = true
    };
    
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    
    TEST_ASSERT_NOT_NULL(strstr(result, "::file-selector-button"));
    
    free(result);
}


void test_refinements_vendor_prefixes_and_pseudos(void) {
    const char *css = 
        "@-webkit-keyframes progress-bar-stripes{0%{background-position-x:1rem}}"
        "::-moz-focus-inner{padding: 0; border-style: none}"
        "::-webkit-search-decoration{-webkit-appearance: none}"
        "::-webkit-color-swatch-wrapper{padding: 0}";
        
    // Usage setup: NO buttons, NO inputs of type search/color, NO animations used
    const char *used[] = {"some-class"}; 
    
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 1,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_STRICT,
        .remove_unused_keyframes = true,
        .remove_form_pseudoelements = true
    };

    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    
    // All should be removed
    TEST_ASSERT_NULL(strstr(result, "progress-bar-stripes"));
    TEST_ASSERT_NULL(strstr(result, "-moz-focus-inner"));
    TEST_ASSERT_NULL(strstr(result, "-webkit-search-decoration"));
    TEST_ASSERT_NULL(strstr(result, "-webkit-color-swatch-wrapper"));
    
    free(result);
}

void run_optimization_tests(void) {
    RUN_TEST(test_remove_unused_keyframes);
    RUN_TEST(test_remove_form_pseudoelements_without_forms);
    RUN_TEST(test_keep_form_pseudoelements_with_forms);
    RUN_TEST(test_refinements_vendor_prefixes_and_pseudos);
}
