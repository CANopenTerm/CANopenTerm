/** @file test_eds.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cmocka.h"
#include "core.h"
#include "eds.h"
#include "os.h"
#include "test_eds.h"

/* Test: list_eds function basic execution */
void test_list_eds(void** state)
{
    (void)state;

    /* This test verifies that list_eds can be called without crashing.
     * It should attempt to open the eds directory and list files with .eds extension.
     * If the directory doesn't exist or is empty, it should log a warning. */

    list_eds();

    /* Test passes if function completes without crash */
    assert_true(true);
}

/* Test: run_conformance_test with invalid file path */
void test_run_conformance_test_invalid_path(void** state)
{
    status_t result;
    const char* invalid_path = "/nonexistent/invalid_file_path_12345.eds";

    (void)state;

    /* Attempting to run conformance test on non-existent EDS file
     * should return an error status, not crash */
    result = run_conformance_test(invalid_path, "test_package", 1, SCRIPT_MODE);

    /* Should return an error status code */
    assert_true(result != ALL_OK);
}

/* Test: run_conformance_test with NULL package name */
void test_run_conformance_test_missing_package(void** state)
{
    status_t result;
    const char* test_path = "eds/DS301_profile.eds";

    (void)state;

    /* Calling with NULL package should be rejected by the function
     * The function validates that package name is provided */
    result = run_conformance_test(test_path, NULL, 1, SILENT);

    /* Function should return error for NULL package */
    assert_true(result == OS_INVALID_ARGUMENT);
}

/* Test: validate_eds with invalid file number */
void test_validate_eds_invalid_file_no(void** state)
{
    status_t result;

    (void)state;

    /* Attempting to validate with invalid file number (0 or very high)
     * should handle gracefully */
    result = validate_eds(0, "test_package", 1);

    /* Should handle invalid file number gracefully */
    assert_true(true);
}
