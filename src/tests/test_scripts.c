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
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "cmocka.h"
#include "core.h"
#include "os.h"
#include "scripts.h"
#include "test_scripts.h"

static int stdout_fd_backup;
static int stdout_fd;

static void assert_files_equal(const char *file1_path, const char *file2_path);
static void convert_crlf_to_lf(const char* file_path);
static void redirect_stdout_to_file(const char* filename);
static void restore_stdout(void);
static void test_python_script(const char* basename);

void test_has_valid_extension(void **state)
{
    (void)state;

    assert_true(has_valid_extension("exceptional_script.lua")  == IS_TRUE);
    assert_true(has_valid_extension("mediocre_script.py")      == IS_TRUE);
    assert_true(has_valid_extension("totally_unsupported.tcl") == IS_FALSE);
}

void test_lua(void** state)
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

static void assert_files_equal(const char* file1_path, const char* file2_path)
{
    FILE* file1, *file2;
    int ch1, ch2;

    file1 = fopen(file1_path, "rb");
    if (file1 == NULL)
    {
        fail_msg("Failed to open %s for reading.", file1_path);
    }

    file2 = fopen(file2_path, "rb");
    if (file2 == NULL)
    {
        fclose(file1);
        fail_msg("Failed to open %s for reading.", file2_path);
    }

    do
    {
        ch1 = fgetc(file1);
        ch2 = fgetc(file2);

        if (ch1 != ch2)
        {
            fclose(file1);
            fclose(file2);
            fail_msg("Files %s and %s are not equal.", file1_path, file2_path);
        }
    } while (ch1 != EOF && ch2 != EOF);

    /* Check if both files reached EOF, otherwise they are of different sizes. */
    if (ch1 != ch2)
    {
        fclose(file1);
        fclose(file2);
        fail_msg("Files %s and %s have different sizes.", file1_path, file2_path);
    }

    fclose(file1);
    fclose(file2);
}

static void convert_crlf_to_lf(const char* file_path)
{
    FILE* file;
    FILE* temp_file;
    char  temp_file_path[1024];
    int   ch;
    int   next_ch;

    file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("Failed to open file for reading");
        return;
    }

    /* Create a temporary file to write the converted content. */
    snprintf(temp_file_path, sizeof(temp_file_path), "%s.tmp", file_path);
    temp_file = fopen(temp_file_path, "wb");
    if (temp_file == NULL)
    {
        perror("Failed to open temporary file for writing");
        fclose(file);
        return;
    }

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\r')
        {
            next_ch = fgetc(file);
            if (next_ch != '\n')
            {
                ungetc(next_ch, file);
            }
            fputc('\n', temp_file);
        }
        else
        {
            fputc(ch, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    /* Replace the original file with the converted file */
    if (remove(file_path) != 0)
    {
        perror("Failed to remove original file");
        return;
    }
    if (rename(temp_file_path, file_path) != 0)
    {
        perror("Failed to rename temporary file");
    }
}

static void redirect_stdout_to_file(const char* filename)
{
    fflush(stdout);
    stdout_fd_backup = dup(fileno(stdout));
    if (NULL == freopen(filename, "w", stdout))
    {
        perror("Failed to redirect stdout to file");
        exit(EXIT_FAILURE);
     }
}

static void restore_stdout(void)
{
    fflush(stdout);
    dup2(stdout_fd_backup, fileno(stdout));
    close(stdout_fd_backup);
}

static void test_python_script(const char* script_path)
{
    core_t core = { 0 };

    scripts_init(&core);
    run_script(script_path, &core);
    scripts_deinit(&core);
}
