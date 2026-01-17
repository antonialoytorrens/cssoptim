#include "unity.h"
#include <stdio.h>

/* Forward declarations of test runners */
void run_css_tests(void);
void run_html_tests(void);
void run_arg_tests(void);
void run_integration_tests(void);
void run_mode_tests(void);
void run_optimization_tests(void);

#include <physfs.h>

void setUp(void) {
    if (!PHYSFS_isInit()) {
        if (!PHYSFS_init("test_runner")) {
            TEST_FAIL_MESSAGE("Could not initialize PhysFS");
        }
    }

    // Mount current working directory
    if (!PHYSFS_mount(".", NULL, 1)) {
        TEST_FAIL_MESSAGE("Could not mount CWD");
    }
}

void tearDown(void) {
    // We could deinit here, but usually we deinit at the very end of main
    // However, Unity calls this after every test.
    // If we want to keep it simple, we can just leave it empty or deinit.
    // PHYSFS_deinit();
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
    
    PHYSFS_deinit();
    return UNITY_END();
}
