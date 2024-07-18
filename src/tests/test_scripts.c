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

static int stdout_fd_backup;

static void compare_files(const char* file1_path, const char* file2_path);
static void redirect_stdout_to_file(const char* filename);
static void restore_stdout(void);
static void test_picoc_script(const char* basename);

void test_has_valid_extension(void **state)
{
    (void)state;

    assert_true(has_valid_extension("exceptional_script.lua") == IS_TRUE);
    assert_true(has_valid_extension("mediocre_script.py")     == IS_FALSE);
    assert_true(has_valid_extension("because_why_not.c")      == IS_TRUE);
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

void test_picoc_00_assignment(void** state)
{
    test_picoc_script("00_assignment");
}

void test_picoc_01_comment(void** state)
{
    test_picoc_script("01_comment");
}

void test_picoc_02_printf(void** state)
{
    test_picoc_script("02_printf");
}

void test_picoc_03_struct(void** state)
{
    test_picoc_script("03_struct");
}

void test_picoc_04_for(void** state)
{
    test_picoc_script("04_for");
}

void test_picoc_05_array(void** state)
{
    test_picoc_script("05_array");
}

void test_picoc_06_case(void** state)
{
    test_picoc_script("06_case");
}

void test_picoc_07_function(void** state)
{
    test_picoc_script("07_function");
}

void test_picoc_08_while(void** state)
{
    test_picoc_script("08_while");
}

void test_picoc_09_do_while(void** state)
{
    test_picoc_script("09_do_while");
}

void test_picoc_10_pointer(void** state)
{
    test_picoc_script("10_pointer");
}

void test_picoc_11_precedence(void** state)
{
    test_picoc_script("11_precedence");
}

void test_picoc_12_hashdefine(void** state)
{
    test_picoc_script("12_hashdefine");
}

void test_picoc_13_integer_literals(void** state)
{
    test_picoc_script("13_integer_literals");
}

void test_picoc_14_if(void** state)
{
    test_picoc_script("14_if");
}

void test_picoc_15_recursion(void** state)
{
    test_picoc_script("15_recursion");
}

void test_picoc_16_nesting(void** state)
{
    test_picoc_script("16_nesting");
}

void test_picoc_17_enum(void** state)
{
    test_picoc_script("17_enum");
}

void test_picoc_18_include(void** state)
{
    test_picoc_script("18_include");
}

void test_picoc_19_pointer_arithmetic(void** state)
{
    test_picoc_script("19_pointer_arithmetic");
}

void test_picoc_20_pointer_comparison(void** state)
{
    test_picoc_script("20_pointer_comparison");
}

void test_picoc_21_char_array(void** state)
{
    test_picoc_script("21_char_array");
}

void test_picoc_22_floating_point(void** state)
{
    test_picoc_script("22_floating_point");
}

void test_picoc_23_type_coercion(void** state)
{
    test_picoc_script("23_type_coercion");
}

void test_picoc_24_math_library(void** state)
{
    test_picoc_script("24_math_library");
}

void test_picoc_25_quicksort(void** state)
{
    test_picoc_script("25_quicksort");
}

void test_picoc_26_character_constants(void** state)
{
    test_picoc_script("26_character_constants");
}

void test_picoc_27_sizeof(void** state)
{
    test_picoc_script("27_sizeof");
}

void test_picoc_28_strings(void** state)
{
    test_picoc_script("28_strings");
}

void test_picoc_29_array_address(void** state)
{
    test_picoc_script("29_array_address");
}

void test_picoc_30_hanoi(void** state)
{
    test_picoc_script("30_hanoi");
}

void test_picoc_31_args(void** state)
{
    test_picoc_script("31_args");
}

void test_picoc_32_led(void** state)
{
    test_picoc_script("32_led");
}

void test_picoc_33_ternary_op(void** state)
{
    test_picoc_script("33_ternary_op");
}

void test_picoc_34_array_assignment(void** state)
{
    test_picoc_script("34_array_assignment");
}

void test_picoc_35_sizeof(void** state)
{
    test_picoc_script("35_sizeof");
}

void test_picoc_36_array_initializers(void** state)
{
    test_picoc_script("36_array_initializers");
}

void test_picoc_37_sprintf(void** state)
{
    test_picoc_script("37_sprintf");
}

void test_picoc_38_multiple_array_index(void** state)
{
    test_picoc_script("38_multiple_array_index");
}

void test_picoc_39_typedef(void** state)
{
    test_picoc_script("39_typedef");
}

void test_picoc_40_stdio(void** state)
{
    test_picoc_script("40_stdio");
}

void test_picoc_41_hashif(void** state)
{
    test_picoc_script("41_hashif");
}

void test_picoc_43_void_param(void** state)
{
    test_picoc_script("43_void_param");
}

void test_picoc_44_scoped_declarations(void** state)
{
    test_picoc_script("44_scoped_declarations");
}

void test_picoc_45_empty_for(void** state)
{
    test_picoc_script("45_empty_for");
}

void test_picoc_47_switch_return(void** state)
{
    test_picoc_script("47_switch_return");
}

void test_picoc_48_nested_break(void** state)
{
    test_picoc_script("48_nested_break");
}

void test_picoc_49_bracket_evaluation(void** state)
{
    test_picoc_script("49_bracket_evaluation");
}

void test_picoc_50_logical_second_arg(void** state)
{
    test_picoc_script("50_logical_second_arg");
}

void test_picoc_51_static(void** state)
{
    test_picoc_script("51_static");
}

void test_picoc_52_unnamed_enum(void** state)
{
    test_picoc_script("52_unnamed_enum");
}

void test_picoc_54_goto(void** state)
{
    test_picoc_script("54_goto");
}

void test_picoc_55_array_initializer(void** state)
{
    test_picoc_script("55_array_initializer");
}

void test_picoc_56_cross_structure(void** state)
{
    test_picoc_script("56_cross_structure");
}

void test_picoc_57_macro_bug(void** state)
{
    test_picoc_script("57_macro_bug");
}

void test_picoc_58_return_outside(void** state)
{
    test_picoc_script("58_return_outside");
}

void test_picoc_59_break_before_loop(void** state)
{
    test_picoc_script("59_break_before_loop");
}

void test_picoc_60_local_vars(void** state)
{
    test_picoc_script("60_local_vars");
}

void test_picoc_61_initializers(void** state)
{
    test_picoc_script("61_initializers");
}

void test_picoc_62_float(void** state)
{
    test_picoc_script("62_float");
}

void test_picoc_63_typedef(void** state)
{
    test_picoc_script("63_typedef");
}

void test_picoc_64_double_prefix_op(void** state)
{
    test_picoc_script("64_double_prefix_op");
}

void test_picoc_65_typeless(void** state)
{
    test_picoc_script("65_typeless");
}

void test_picoc_66_printf_undefined(void** state)
{
    test_picoc_script("66_printf_undefined");
}

void test_picoc_67_macro_crash(void** state)
{
    test_picoc_script("67_macro_crash");
}

void test_picoc_68_return(void** state)
{
    test_picoc_script("68_return");
}

static void compare_files(const char* file1_path, const char* file2_path)
{
    FILE* file1;
    FILE* file2;
    char* buffer1;
    char* buffer2;
    long  file1_size;
    long  file2_size;

    file1 = fopen(file1_path, "rb");
    assert_non_null(file1);

    file2 = fopen(file2_path, "rb");
    assert_non_null(file2);

    fseek(file1, 0, SEEK_END);
    file1_size = ftell(file1);
    rewind(file1);

    fseek(file2, 0, SEEK_END);
    file2_size = ftell(file2);
    rewind(file2);

    buffer1 = (char*)malloc(file1_size);
    buffer2 = (char*)malloc(file2_size);

    fread(buffer1, 1, file1_size, file1);
    fread(buffer2, 1, file2_size, file2);

    assert_memory_equal(buffer1, buffer2, file1_size);

    free(buffer1);
    free(buffer2);
    fclose(file1);
    fclose(file2);
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

static void test_picoc_script(const char* basename)
{
    core_t core = { 0 };
    char   result_file_path[256];
    char   temp_file_path[256];
    char   expect_file_path[256];
    char   script_name[256];
    int    original_stdout_fd;

    snprintf(result_file_path, sizeof(result_file_path), "tests/%s.result", basename);
    snprintf(expect_file_path, sizeof(expect_file_path), "tests/%s.expect", basename);

    scripts_init(&core);

    redirect_stdout_to_file(result_file_path);
    snprintf(script_name, sizeof(script_name), "%s.c", basename);
    run_script(script_name, &core);
    compare_files(result_file_path, expect_file_path);
    restore_stdout();

    scripts_deinit(&core);
}
