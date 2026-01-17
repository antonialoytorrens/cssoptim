#include "unity.h"
#include <stdio.h>

/* Forward declarations of test runners */
void run_css_tests(void);
void run_html_tests(void);
void run_arg_tests(void);
void run_integration_tests(void);
void run_mode_tests(void);
void run_optimization_tests(void);

void setUp(void) {
  // Standard setup
}

void tearDown(void) {
  // Standard teardown
}

int main(void) {
  UNITY_BEGIN();

  // RUN_TEST(test_function_name);
  run_css_tests();
  run_html_tests();
  run_arg_tests();
  run_integration_tests();
  run_mode_tests();
  run_optimization_tests();

  return UNITY_END();
}
