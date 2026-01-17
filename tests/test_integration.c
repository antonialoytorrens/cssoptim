#include "unity.h"
#include <physfs.h>
#include <stdlib.h>
#include <string.h>

#include "../src/html_scan.h"
#include "../src/css_proc.h"

static char* read_physfs_file(const char *path, PHYSFS_sint64 *out_len) {
    if (!PHYSFS_exists(path)) return NULL;
    PHYSFS_File *f = PHYSFS_openRead(path);
    if (!f) return NULL;
    
    PHYSFS_sint64 len = PHYSFS_fileLength(f);
    char *buf = malloc(len + 1);
    if (!buf) {
        PHYSFS_close(f);
        return NULL;
    }
    
    PHYSFS_readBytes(f, buf, len);
    buf[len] = '\0';
    if (out_len) *out_len = len;
    PHYSFS_close(f);
    return buf;
}

void test_physfs_setup_and_read_fixture(void) {
    // Init
    setUp();

    // Try to read tests/fixtures/test.css
    const char *path = "tests/fixtures/test.css";
    TEST_ASSERT_TRUE_MESSAGE(PHYSFS_exists(path), "test.css fixture not found via PhysFS");

    PHYSFS_File *f = PHYSFS_openRead(path);
    TEST_ASSERT_NOT_NULL(f);
    
    PHYSFS_sint64 len = PHYSFS_fileLength(f);
    TEST_ASSERT_GREATER_THAN(0, len);
    
    char *buf = malloc(len + 1);
    TEST_ASSERT_NOT_NULL(buf);
    
    PHYSFS_sint64 read_len = PHYSFS_readBytes(f, buf, len);
    TEST_ASSERT_EQUAL(len, read_len);
    buf[len] = '\0';
    
    // Check content (simple check)
    TEST_ASSERT_NOT_NULL(strstr(buf, ".used"));
    
    free(buf);
    PHYSFS_close(f);
    
    // Cleanup handled by runner or teardown? 
    // Usually only Deinit at very end.
}

void test_manual_minified(void) {
    const char *css = ".d-flex{display:flex!important}.flex-column{flex-direction:column!important}.h-100{height:100%!important}.bg-turquoise{background-color:turquoise!important}.unused{color:red}";
    const char *used[] = {"d-flex", "flex-column", "h-100", "bg-turquoise"};
    
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 4,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_STRICT
    };
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    
    TEST_ASSERT_NOT_NULL(strstr(result, ".d-flex"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".flex-column"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".h-100"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".bg-turquoise"));
    TEST_ASSERT_NULL(strstr(result, ".unused"));
    
    free(result);
}

void test_manual_minified_media_queries(void) {
    const char *css = ".d-flex{display:flex!important}.flex-column{flex-direction:column!important}.h-100{height:100%!important}.bg-turquoise{background-color:turquoise!important}.unused{color:red}@media{.mediatest{color:red}}";
    const char *used[] = {"d-flex", "flex-column", "h-100", "bg-turquoise", "mediatest"};
    
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 5,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_STRICT
    };
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    
    TEST_ASSERT_NOT_NULL(strstr(result, ".d-flex"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".flex-column"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".h-100"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".bg-turquoise"));
    TEST_ASSERT_NULL(strstr(result, ".unused"));
    
    free(result);
}

void test_complex_filtering(void) {
    const char *css = 
        "@media (min-width: 500px) {\n"
        "    .used { color: var(--used-var); animation: slide 1s; }\n"
        "    .unused { color: red; }\n"
        "}\n"
        ":root { --used-var: blue; --unused-var: red; }\n"
        "@keyframes slide { from { opacity: 0; } }\n"
        "@keyframes unused-anim { from { opacity: 0; } }";
    
    const char *used[] = {"used"};
    
    // Explicitly using length
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 1,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_SAFE,
        .remove_unused_keyframes = true
    };
    char *result = css_optimize(css, strlen(css), &config);
    TEST_ASSERT_NOT_NULL(result);
    // Print for debug
    // printf("Result: %s\n", result);
    
    // Assertions
    TEST_ASSERT_NOT_NULL(strstr(result, ".used"));
    TEST_ASSERT_NOT_NULL(strstr(result, "--used-var")); // Variable preserved
    TEST_ASSERT_NOT_NULL(strstr(result, "slide"));      // Animation preserved
    
    // Negative assertions
    TEST_ASSERT_NULL(strstr(result, ".unused"));
    TEST_ASSERT_NULL(strstr(result, "--unused-var"));
    TEST_ASSERT_NULL(strstr(result, "unused-anim"));
    
    free(result);
}


void test_bootstrap_reduction(void) {
    PHYSFS_sint64 len;
    char *css = read_physfs_file("tests/fixtures/bootstrap.css", &len);
    TEST_ASSERT_NOT_NULL_MESSAGE(css, "Could not read bootstrap.css");
    
    // Simulate bootstrap.html classes
    const char *used[] = {"d-flex", "flex-column", "h-100", "bg-turquoise"};
    
    OptimizerConfig config = {
        .used_classes = used,
        .class_count = 4,
        .used_tags = NULL,
        .tag_count = 0,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_STRICT
    };
    char *result = css_optimize(css, len, &config);
    TEST_ASSERT_NOT_NULL(result);
    
    // Should contain used classes
    TEST_ASSERT_NOT_NULL(strstr(result, ".d-flex"));
    TEST_ASSERT_NOT_NULL(strstr(result, ".h-100"));
    
    // Should NOT contain unused classes
    char *found = strstr(result, ".accordion-button");
    if (found) {
        printf("DEBUG: Found residual .accordion-button at: %.100s\n", found);
    }
    TEST_ASSERT_NULL_MESSAGE(found, "Bootstrap accordion unused class should be removed");
    TEST_ASSERT_NULL_MESSAGE(strstr(result, ".btn-primary"), "Bootstrap btn-primary unused class should be removed");
    
    free(result);
    free(css);
}

void test_html_tag_removal(void) {
    // 1. Read htmlrmtest.html content to scan it
    PHYSFS_sint64 h_len;
    char *html_content = read_physfs_file("tests/fixtures/htmlrmtest.html", &h_len);
    TEST_ASSERT_NOT_NULL(html_content);
    
    // Scan it
    string_list_t used_classes;
    string_list_init(&used_classes);
    string_list_t used_tags;
    string_list_init(&used_tags);
    
    scan_html(html_content, h_len, &used_classes, &used_tags, NULL);
    
    // Verify scan results first
    TEST_ASSERT_TRUE(string_list_contains(&used_tags, "h2"));
    TEST_ASSERT_TRUE(string_list_contains(&used_tags, "h3"));
    TEST_ASSERT_TRUE(string_list_contains(&used_classes, "class1"));
    TEST_ASSERT_TRUE(string_list_contains(&used_classes, "class2"));
    TEST_ASSERT_TRUE(string_list_contains(&used_classes, "class3"));
    
    // 2. Read htmlrmtest.css
    PHYSFS_sint64 c_len;
    char *css_content = read_physfs_file("tests/fixtures/htmlrmtest.css", &c_len);
    TEST_ASSERT_NOT_NULL(css_content);
    
    // 3. Optimize
    OptimizerConfig config = {
        .used_classes = (const char**)used_classes.items,
        .class_count = used_classes.count,
        .used_tags = (const char**)used_tags.items,
        .tag_count = used_tags.count,
        .used_attrs = NULL,
        .attr_count = 0,
        .mode = LXB_CSS_OPTIM_MODE_STRICT
    };
    char *result = css_optimize(css_content, c_len, &config);
    
    TEST_ASSERT_NOT_NULL(result);
    printf("Result tag removal: %s\n", result);
    
    // 4. Assertions
    // h1 should be removed
    TEST_ASSERT_NULL(strstr(result, "h1")); 
    
    // h2 should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "h2"));
    
    // class1 should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "class1"));
    
    // Cleanup
    free(result);
    free(css_content);
    free(html_content);
    string_list_destroy(&used_classes);
    string_list_destroy(&used_tags);
}

void test_attr_and_pseudo(void) {
    // 1. Read attrtest.html
    PHYSFS_sint64 h_len;
    char *html = read_physfs_file("tests/fixtures/attrtest.html", &h_len);
    TEST_ASSERT_NOT_NULL(html);
    
    string_list_t classes, tags, attrs;
    string_list_init(&classes);
    string_list_init(&tags);
    string_list_init(&attrs);
    
    scan_html(html, h_len, &classes, &tags, &attrs);
    
    // Check attributes
    TEST_ASSERT_TRUE(string_list_contains(&attrs, "role=button"));
    TEST_ASSERT_TRUE(string_list_contains(&attrs, "data-custom=test"));
    TEST_ASSERT_FALSE(string_list_contains(&attrs, "type=submit"));
    
    // 2. Read attrtest.css
    PHYSFS_sint64 c_len;
    char *css = read_physfs_file("tests/fixtures/attrtest.css", &c_len);
    TEST_ASSERT_NOT_NULL(css);
    
    // 3. Optimize
    OptimizerConfig config = {
        .used_classes = (const char**)classes.items,
        .class_count = classes.count,
        .used_tags = (const char**)tags.items,
        .tag_count = tags.count,
        .used_attrs = (const char**)attrs.items,
        .attr_count = attrs.count,
        .mode = LXB_CSS_OPTIM_MODE_CONSERVATIVE
    };
    char *result = css_optimize(css, c_len, &config);
    
    TEST_ASSERT_NOT_NULL(result);
    printf("Result Attr/Pseudo:\n%s\n", result);
    
    // 4. Assertions
    // Charset MUST have semicolon
    TEST_ASSERT_NOT_NULL(strstr(result, "@charset \"UTF-8\";"));
    
    // role="button" should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "[role=\"button\"]"));
    
    // type="submit" should be removed
    TEST_ASSERT_NULL(strstr(result, "[type=\"submit\"]"));
    
    // data-custom="test" should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "[data-custom=\"test\"]"));
    
    // Browser pseudos should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "-webkit-input-placeholder"));
    TEST_ASSERT_NOT_NULL(strstr(result, "-moz-placeholder"));
    
    // button:focus should be kept
    TEST_ASSERT_NOT_NULL(strstr(result, "button:focus"));
    
    // span:focus should be removed (no span in HTML)
    TEST_ASSERT_NULL(strstr(result, "span:focus"));
    
    // Cleanup
    free(result);
    free(css);
    free(html);
    string_list_destroy(&classes);
    string_list_destroy(&tags);
    string_list_destroy(&attrs);
}

void run_integration_tests(void) {
    RUN_TEST(test_physfs_setup_and_read_fixture);
    RUN_TEST(test_manual_minified);
    RUN_TEST(test_manual_minified_media_queries);
    RUN_TEST(test_complex_filtering);
    RUN_TEST(test_bootstrap_reduction);
    RUN_TEST(test_html_tag_removal);
    RUN_TEST(test_attr_and_pseudo);
}
