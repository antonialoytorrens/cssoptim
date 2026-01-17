#include "cssoptim/scanner.h"
#include "unity.h"
#include <string.h>

void test_class_list_basic(void) {
  string_list_t *list = string_list_create();
  TEST_ASSERT_NOT_NULL(list);

  string_list_add(list, "foo");
  string_list_add(list, "bar");

  TEST_ASSERT_EQUAL(2, string_list_count(list));
  TEST_ASSERT_TRUE(string_list_contains(list, "foo"));
  TEST_ASSERT_TRUE(string_list_contains(list, "bar"));
  TEST_ASSERT_FALSE(string_list_contains(list, "baz"));

  string_list_destroy(list);
}

void test_scan_html_basic(void) {
  const char *html =
      "<html><body><div class=\"foo bar\">Hello</div></body></html>";
  string_list_t *list = string_list_create();
  TEST_ASSERT_NOT_NULL(list);

  scan_html(html, strlen(html), list, NULL, NULL);

  TEST_ASSERT_TRUE(string_list_contains(list, "foo"));
  TEST_ASSERT_TRUE(string_list_contains(list, "bar"));

  string_list_destroy(list);
}

void test_scan_js_basic(void) {
  const char *js = "var x = 'foo'; let y = \"bar\"; const z = `baz`;";
  string_list_t *list = string_list_create();
  TEST_ASSERT_NOT_NULL(list);

  scan_js(js, strlen(js), list);

  TEST_ASSERT_TRUE(string_list_contains(list, "foo"));
  TEST_ASSERT_TRUE(string_list_contains(list, "bar"));
  TEST_ASSERT_TRUE(string_list_contains(list, "baz"));

  string_list_destroy(list);
}

void run_html_tests(void) {
  RUN_TEST(test_class_list_basic);
  RUN_TEST(test_scan_html_basic);
  RUN_TEST(test_scan_js_basic);
}
