/** @file test_buffer.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cmocka.h"
#include "codb2json.h"
#include "os.h"
#include "test_codb.h"

static void assert_files_equal(const char* file1_path, const char* file2_path);

void test_codb2json(void** state)
{
    (void)state;

    assert_true(codb2json(2, (char*[]){"codb2json", "tests/test.codb"}, IS_FALSE) == 0);
    assert_files_equal("tests/test.json", "tests/test.json.expected");
}

static void assert_files_equal(const char* file1_path, const char* file2_path)
{
    FILE_t *file1, *file2;
    char    line1[256], line2[256];

    file1 = os_fopen(file1_path, "rb");
    if (file1 == NULL)
    {
        fail_msg("Failed to open %s for reading.", file1_path);
    }

    file2 = os_fopen(file2_path, "rb");
    if (file2 == NULL)
    {
        os_fclose(file1);
        fail_msg("Failed to open %s for reading.", file2_path);
    }

    while (fgets(line1, sizeof(line1), file1) != NULL && fgets(line2, sizeof(line2), file2) != NULL)
    {
        if (strcmp(line1, line2) != 0)
        {
            os_fclose(file1);
            os_fclose(file2);
            fail_msg("Files %s and %s are not equal.", file1_path, file2_path);
        }
    }

    os_fclose(file1);
    os_fclose(file2);
}
