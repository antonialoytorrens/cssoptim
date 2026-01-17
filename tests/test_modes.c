#include "cssoptim/optimizer.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void test_universal_selectors(void) {
  const char *css = "* { margin: 0; } .used { color: red; }";
  const char *used[] = {"used"};

  // Strict mode: * is usually kept because it matches everything, but let's
  // test our implementation
  OptimizerConfig config_strict = {.used_classes = used,
                                   .class_count = 1,
                                   .used_tags = NULL,
                                   .tag_count = 0,
                                   .used_attrs = NULL,
                                   .attr_count = 0,
                                   .mode = LXB_CSS_OPTIM_MODE_STRICT};
  char *res_strict = css_optimize(css, strlen(css), &config_strict);
  TEST_ASSERT_NOT_NULL(res_strict);
  TEST_ASSERT_NOT_NULL(strstr(res_strict, "*"));
  free(res_strict);

  // Safe mode: * must be kept
  OptimizerConfig config_safe = {.used_classes = used,
                                 .class_count = 1,
                                 .used_tags = NULL,
                                 .tag_count = 0,
                                 .used_attrs = NULL,
                                 .attr_count = 0,
                                 .mode = LXB_CSS_OPTIM_MODE_SAFE};
  char *res_safe = css_optimize(css, strlen(css), &config_safe);
  TEST_ASSERT_NOT_NULL(res_safe);
  TEST_ASSERT_NOT_NULL(strstr(res_safe, "*"));
  free(res_safe);
}

void test_pseudo_elements_modes(void) {
  const char *css = "div::before { content: ''; } span::after { color: blue; }";
  const char *used_tags[] = {"div"};

  // Strict mode: span is not used, so span::after should be removed
  OptimizerConfig config = {.used_classes = NULL,
                            .class_count = 0,
                            .used_tags = used_tags,
                            .tag_count = 1,
                            .used_attrs = NULL,
                            .attr_count = 0,
                            .mode = LXB_CSS_OPTIM_MODE_STRICT};
  char *result = css_optimize(css, strlen(css), &config);
  TEST_ASSERT_NOT_NULL(result);
  if (strstr(result, "span::after")) {
    printf("DEBUG: result contained 'span::after': %s\n", result);
  }
  TEST_ASSERT_NOT_NULL(strstr(result, "div::before"));
  TEST_ASSERT_NULL(strstr(result, "span::after"));
  free(result);
}

void test_complex_selectors_logic(void) {
  const char *css =
      ".parent .child { color: red; } .unused .child { color: blue; }";
  const char *used[] = {"parent", "child"};

  OptimizerConfig config = {.used_classes = used,
                            .class_count = 2,
                            .used_tags = NULL,
                            .tag_count = 0,
                            .used_attrs = NULL,
                            .attr_count = 0,
                            .mode = LXB_CSS_OPTIM_MODE_STRICT};
  char *result = css_optimize(css, strlen(css), &config);
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_NOT_NULL(strstr(result, ".parent .child"));
  TEST_ASSERT_NULL(strstr(result, ".unused .child"));
  free(result);
}

void test_same_class_multiple_rules(void) {
  const char *css = ".d-flex { display: flex; } .d-flex { margin: 0; }";
  const char *used[] = {"d-flex"};

  OptimizerConfig config = {.used_classes = used,
                            .class_count = 1,
                            .used_tags = NULL,
                            .tag_count = 0,
                            .used_attrs = NULL,
                            .attr_count = 0,
                            .mode = LXB_CSS_OPTIM_MODE_STRICT};
  char *result = css_optimize(css, strlen(css), &config);
  TEST_ASSERT_NOT_NULL(result);
  // Should contain both rules
  int count = 0;
  char *p = result;
  while ((p = strstr(p, ".d-flex"))) {
    count++;
    p += 7;
  }
  TEST_ASSERT_EQUAL(2, count);
  free(result);
}

void test_empty_classes_logic(void) {
  const char *css = ".used { color: red; }";
  const char *used[] = {"used", ""}; // empty class name

  OptimizerConfig config = {.used_classes = used,
                            .class_count = 2,
                            .used_tags = NULL,
                            .tag_count = 0,
                            .used_attrs = NULL,
                            .attr_count = 0,
                            .mode = LXB_CSS_OPTIM_MODE_STRICT};
  char *result = css_optimize(css, strlen(css), &config);
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_NOT_NULL(strstr(result, ".used"));
  free(result);
}

void run_mode_tests(void) {
  RUN_TEST(test_universal_selectors);
  RUN_TEST(test_pseudo_elements_modes);
  RUN_TEST(test_complex_selectors_logic);
  RUN_TEST(test_same_class_multiple_rules);
  RUN_TEST(test_empty_classes_logic);
}
