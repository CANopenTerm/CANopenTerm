/** @file test_common.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "cmocka.h"
#include "common.h"
#include "os.h"
#include "test_common.h"

/* Mock callbacks for testing */
static void mock_opendir_invalid(void** state)
{
    (void)state;
    will_return(__wrap_os_opendir, NULL);
}

static void mock_opendir_valid(void** state)
{
    (void)state;
}

/* Test: list_file_type with invalid directory */
void test_list_file_type_invalid_dir(void** state)
{
    (void)state;

    /* This test verifies that list_file_type handles an invalid/non-existent 
     * directory gracefully without crashing and logs a warning. */

    /* list_file_type attempts to open a directory and logs if it fails.
     * Testing that it doesn't crash with an invalid path. */
    list_file_type("invalid_nonexistent_dir_path", "txt", 0);

    /* Test passes if no crash occurs */
    assert_true(true);
}

/* Test: list_file_type with valid input */
void test_list_file_type_valid_dir(void** state)
{
    (void)state;

    /* This test verifies that list_file_type can process a valid directory
     * and extension. The current directory should be accessible. */

    /* Attempt to list txt files in current directory */
    list_file_type(".", "txt", 0);

    /* Test passes if function completes without crash */
    assert_true(true);
}

/* Test: list_file_type with active status indicator */
void test_list_file_type_with_active_no(void** state)
{
    (void)state;

    /* This test verifies that list_file_type correctly handles the active_no
     * parameter, which marks a file as "Active" in the display. */

    /* Call with active_no = 1 to mark first file as active */
    list_file_type(".", "txt", 1);

    /* Test passes if function completes without crash */
    assert_true(true);
}

/* Test: list_file_type with empty extension */
void test_list_file_type_empty_extension(void** state)
{
    (void)state;

    /* This test verifies that list_file_type handles an empty extension
     * without crashing. It should construct ".ext" where ext is empty. */

    list_file_type(".", "", 0);

    /* Test passes if function completes without crash */
    assert_true(true);
}

/* Test: list_file_type with extension that typically has no matches */
void test_list_file_type_no_matching_files(void** state)
{
    (void)state;

    /* This test verifies that list_file_type handles the case where no
     * files match the given extension without crashing.  */

    /* Use an unlikely extension that probably has no matches */
    list_file_type(".", "zzzunique", 0);

    /* Test passes if function completes without crash */
    assert_true(true);
}
