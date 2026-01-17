#include "unity.h"
#include "../src/args.h"
#include <stddef.h>

void test_args_explicit(void) {
    const char *argv[] = {"prog", "-o", "output.css", "--css", "input.css", "--html", "index.html"};
    int argc = 7;
    css_args_t args = {0};

    int result = parse_args(argc, argv, &args);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_STRING("output.css", args.output_file);
    TEST_ASSERT_EQUAL(1, args.css_file_count);
    TEST_ASSERT_EQUAL_STRING("input.css", args.css_files[0]);
    TEST_ASSERT_EQUAL(1, args.html_file_count);
    TEST_ASSERT_EQUAL_STRING("index.html", args.html_files[0]);
}

void test_args_verbose(void) {
    const char *argv[] = {"prog", "-v", "--css", "style.css"};
    int argc = 4;
    css_args_t args = {0};

    int result = parse_args(argc, argv, &args);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_TRUE(args.verbose);
    TEST_ASSERT_EQUAL(1, args.css_file_count);
}

void run_arg_tests(void) {
    RUN_TEST(test_args_explicit);
    RUN_TEST(test_args_verbose);
}
