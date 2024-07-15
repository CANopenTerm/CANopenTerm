/** @file test_scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include "cmocka.h"
#include "os.h"
#include "scripts.h"
#include "test_scripts.h"

void test_has_valid_extension(void **state)
{
    (void)state;

    assert_true(has_valid_extension("exceptional_script.lua") == IS_TRUE);
    assert_true(has_valid_extension("mediocre_script.py")     == IS_FALSE);
    assert_true(has_valid_extension("because_why_not.c")      == IS_TRUE);
}

void test_run_script(void** state)
{
    core_t core = { 0 };
    FILE*  lua_file;

    (void)state;

    if (mkdir("scripts", 0777) != 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
            return;
        }
    }

    lua_file = fopen("scripts/test.lua", "w+");

    if (lua_file == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return;
    }

    scripts_init(&core);

    fprintf(lua_file, "-- Lua script to test and validate basic Lua functionality\n\n");

    /* Basic Lua operations. */
    fprintf(lua_file, "-- Variable assignment\n");
    fprintf(lua_file, "local x = 10\n");

    fprintf(lua_file, "-- Arithmetic operation\n");
    fprintf(lua_file, "local y = x + 5\n");

    fprintf(lua_file, "-- String concatenation\n");
    fprintf(lua_file, "local message = 'Lua is great!'\n");

    fprintf(lua_file, "-- Table creation and indexing\n");
    fprintf(lua_file, "local t = { 'apple', 'banana', 'cherry' }\n");
    fprintf(lua_file, "local fruit = t[2]\n");

    fprintf(lua_file, "-- Function definition\n");
    fprintf(lua_file, "local function add(a, b)\n");
    fprintf(lua_file, "    return a + b\n");
    fprintf(lua_file, "end\n");

    fprintf(lua_file, "-- Function call\n");
    fprintf(lua_file, "local result = add(x, y)\n");

    fprintf(lua_file, "-- Boolean comparison\n");
    fprintf(lua_file, "local isGreater = x > y\n");

    fprintf(lua_file, "-- Control structure (if-else)\n");
    fprintf(lua_file, "if isGreater then\n");
    fprintf(lua_file, "    result = result * 2\n");
    fprintf(lua_file, "else\n");
    fprintf(lua_file, "    result = result / 2\n");
    fprintf(lua_file, "end\n");

    /* Assertions to validate results. */
    fprintf(lua_file, "-- Assertions to validate results\n");
    fprintf(lua_file, "assert(x == 10, 'Variable x should be 10')\n");
    fprintf(lua_file, "assert(y == 15, 'Variable y should be 15')\n");
    fprintf(lua_file, "assert(message == 'Lua is great!', 'Message should be Lua is great!')\n");
    fprintf(lua_file, "assert(fruit == 'banana', 'Fruit should be banana')\n");
    fprintf(lua_file, "assert(result == 12.5, 'Result should be 12.5')\n");

    fclose(lua_file);
    run_script("test.lua", &core);
    scripts_deinit(&core);
}
