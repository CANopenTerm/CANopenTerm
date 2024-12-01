/** @file tefst_scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef _WIN32
#    include <direct.h>
#    include <io.h>
#    define _mkdir(dir, mode) _mkdir(dir)
#else
#    include <sys/stat.h>
#    include <sys/types.h>
#    define _close close
#    define _dup dup
#    define _dup2 dup2
#    define _mkdir(dir, mode) mkdir(dir, mode)
#endif

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
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
    core_t  core = { 0 };
    FILE_t* lua_file;

    (void)state;

    if (_mkdir("scripts", 0777) != 0)
    {
        if (errno != EEXIST)
        {
            os_fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
            return;
        }
    }

    lua_file = os_fopen("scripts/test.lua", "w+");

    if (lua_file == NULL)
    {
        os_fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return;
    }

    scripts_init(&core);

    os_fprintf(lua_file, "-- Lua script to test and validate basic Lua functionality\n\n");

    /* Basic Lua operations. */
    os_fprintf(lua_file, "-- Variable assignment\n");
    os_fprintf(lua_file, "local x = 10\n");

    os_fprintf(lua_file, "-- Arithmetic operation\n");
    os_fprintf(lua_file, "local y = x + 5\n");

    os_fprintf(lua_file, "-- String concatenation\n");
    os_fprintf(lua_file, "local message = 'Lua is great!'\n");

    os_fprintf(lua_file, "-- Table creation and indexing\n");
    os_fprintf(lua_file, "local t = { 'apple', 'banana', 'cherry' }\n");
    os_fprintf(lua_file, "local fruit = t[2]\n");

    os_fprintf(lua_file, "-- Function definition\n");
    os_fprintf(lua_file, "local function add(a, b)\n");
    os_fprintf(lua_file, "    return a + b\n");
    os_fprintf(lua_file, "end\n");

    os_fprintf(lua_file, "-- Function call\n");
    os_fprintf(lua_file, "local result = add(x, y)\n");

    os_fprintf(lua_file, "-- Boolean comparison\n");
    os_fprintf(lua_file, "local isGreater = x > y\n");

    os_fprintf(lua_file, "-- Control structure (if-else)\n");
    os_fprintf(lua_file, "if isGreater then\n");
    os_fprintf(lua_file, "    result = result * 2\n");
    os_fprintf(lua_file, "else\n");
    os_fprintf(lua_file, "    result = result / 2\n");
    os_fprintf(lua_file, "end\n");

    /* Assertions to validate results. */
    os_fprintf(lua_file, "-- Assertions to validate results\n");
    os_fprintf(lua_file, "assert(x == 10, 'Variable x should be 10')\n");
    os_fprintf(lua_file, "assert(y == 15, 'Variable y should be 15')\n");
    os_fprintf(lua_file, "assert(message == 'Lua is great!', 'Message should be Lua is great!')\n");
    os_fprintf(lua_file, "assert(fruit == 'banana', 'Fruit should be banana')\n");
    os_fprintf(lua_file, "assert(result == 12.5, 'Result should be 12.5')\n");

    os_fclose(lua_file);
    run_script("test.lua", &core);

    scripts_deinit(&core);
}

static void assert_files_equal(const char* file1_path, const char* file2_path)
{
    FILE_t* file1, *file2;
    int ch1, ch2;

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

    do
    {
        ch1 = fgetc(file1);
        ch2 = fgetc(file2);

        if (ch1 != ch2)
        {
            os_fclose(file1);
            os_fclose(file2);
            fail_msg("Files %s and %s are not equal.", file1_path, file2_path);
        }
    } while (ch1 != EOF && ch2 != EOF);

    /* Check if both files reached EOF, otherwise they are of different sizes. */
    if (ch1 != ch2)
    {
        os_fclose(file1);
        os_fclose(file2);
        fail_msg("Files %s and %s have different sizes.", file1_path, file2_path);
    }

    os_fclose(file1);
    os_fclose(file2);
}

static void convert_crlf_to_lf(const char* file_path)
{
    FILE_t* file;
    FILE_t* temp_file;
    char    temp_file_path[1024];
    int     ch;
    int     next_ch;

    file = os_fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("Failed to open file for reading");
        return;
    }

    /* Create a temporary file to write the converted content. */
    os_snprintf(temp_file_path, sizeof(temp_file_path), "%s.tmp", file_path);
    temp_file = fopen(temp_file_path, "wb");
    if (temp_file == NULL)
    {
        perror("Failed to open temporary file for writing");
        os_fclose(file);
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

    os_fclose(file);
    os_fclose(temp_file);

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
    stdout_fd_backup = _dup(_fileno(stdout));
    if (NULL == freopen(filename, "w", stdout))
    {
        perror("Failed to redirect stdout to file");
        exit(EXIT_FAILURE);
     }
}

static void restore_stdout(void)
{
    fflush(stdout);
    _dup2(stdout_fd_backup, _fileno(stdout));
    _close(stdout_fd_backup);
}

static void test_python_script(const char* script_path)
{
    core_t core = { 0 };

    scripts_init(&core);
    run_script(script_path, &core);
    scripts_deinit(&core);
}
