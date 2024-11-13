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
#include "os.h"
#include "scripts.h"
#include "test_scripts.h"

static int stdout_fd_backup;

static void assert_files_equal(const char *file1_path, const char *file2_path);
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
    (void)state;
    test_picoc_script("tests/00_assignment.c");
}

void test_picoc_00_linked_list(void** state)
{
    (void)state;
    test_picoc_script("tests/00_linked_list.c");
}

void test_picoc_01_comment(void** state)
{
    (void)state;
    test_picoc_script("tests/01_comment.c");
}

void test_picoc_02_printf(void** state)
{
    (void)state;
    test_picoc_script("tests/02_printf.c");
}

void test_picoc_03_struct(void** state)
{
    (void)state;
    test_picoc_script("tests/03_struct.c");
}

void test_picoc_04_for(void** state)
{
    (void)state;
    test_picoc_script("tests/04_for.c");
}

void test_picoc_05_array(void** state)
{
    (void)state;
    test_picoc_script("tests/05_array.c");
}

void test_picoc_06_case(void** state)
{
    (void)state;
    test_picoc_script("tests/06_case.c");
}

void test_picoc_07_function(void** state)
{
    (void)state;
    test_picoc_script("tests/07_function.c");
}

void test_picoc_08_while(void** state)
{
    (void)state;
    test_picoc_script("tests/08_while.c");
}

void test_picoc_09_do_while(void** state)
{
    (void)state;
    test_picoc_script("tests/09_do_while.c");
}

void test_picoc_10_pointer(void** state)
{
    (void)state;
    test_picoc_script("tests/10_pointer.c");
}

void test_picoc_11_precedence(void** state)
{
    (void)state;
    test_picoc_script("tests/11_precedence.c");
}

void test_picoc_12_hashdefine(void** state)
{
    (void)state;
    test_picoc_script("tests/12_hashdefine.c");
}

void test_picoc_13_integer_literals(void** state)
{
    (void)state;
    test_picoc_script("tests/13_integer_literals.c");
}

void test_picoc_14_if(void** state)
{
    (void)state;
    test_picoc_script("tests/14_if.c");
}

void test_picoc_15_recursion(void** state)
{
    (void)state;
    test_picoc_script("tests/15_recursion.c");
}

void test_picoc_16_nesting(void** state)
{
    (void)state;
    test_picoc_script("tests/16_nesting.c");
}

void test_picoc_17_enum(void** state)
{
    (void)state;
    test_picoc_script("tests/17_enum.c");
}

void test_picoc_18_include(void** state)
{
    (void)state;
    test_picoc_script("tests/18_include.c");
}

void test_picoc_19_pointer_arithmetic(void** state)
{
    (void)state;
    test_picoc_script("tests/19_pointer_arithmetic.c");
}

void test_picoc_20_pointer_comparison(void** state)
{
    (void)state;
    test_picoc_script("tests/20_pointer_comparison.c");
}

void test_picoc_21_char_array(void** state)
{
    (void)state;
    test_picoc_script("tests/21_char_array.c");
}

void test_picoc_22_floating_point(void** state)
{
    (void)state;
    test_picoc_script("tests/22_floating_point.c");
}

void test_picoc_23_type_coercion(void** state)
{
    (void)state;
    test_picoc_script("tests/23_type_coercion.c");
}

void test_picoc_24_math_library(void** state)
{
    (void)state;
    test_picoc_script("tests/24_math_library.c");
}

void test_picoc_25_quicksort(void** state)
{
    (void)state;
    test_picoc_script("tests/25_quicksort.c");
}

void test_picoc_26_character_constants(void** state)
{
    (void)state;
    test_picoc_script("tests/26_character_constants.c");
}

void test_picoc_27_sizeof(void** state)
{
    (void)state;
    test_picoc_script("tests/27_sizeof.c");
}

void test_picoc_28_strings(void** state)
{
    (void)state;
    test_picoc_script("tests/28_strings.c");
}

void test_picoc_29_array_address(void** state)
{
    (void)state;
    test_picoc_script("tests/29_array_address.c");
}

void test_picoc_30_hanoi(void** state)
{
    (void)state;
    test_picoc_script("tests/30_hanoi.c");
}

void test_picoc_31_args(void** state)
{
    (void)state;
    test_picoc_script("tests/31_args.c");
}

void test_picoc_32_led(void** state)
{
    (void)state;
    test_picoc_script("tests/32_led.c");
}

void test_picoc_33_ternary_op(void** state)
{
    (void)state;
    test_picoc_script("tests/33_ternary_op.c");
}

void test_picoc_34_array_assignment(void** state)
{
    (void)state;
    test_picoc_script("tests/34_array_assignment.c");
}

void test_picoc_35_sizeof(void** state)
{
    (void)state;
    test_picoc_script("tests/35_sizeof.c");
}

void test_picoc_36_array_initializers(void** state)
{
    (void)state;
    test_picoc_script("tests/36_array_initializers.c");
}

void test_picoc_37_sprintf(void** state)
{
    (void)state;
    test_picoc_script("tests/37_sprintf.c");
}

void test_picoc_38_multiple_array_index(void** state)
{
    (void)state;
    test_picoc_script("tests/38_multiple_array_index.c");
}

void test_picoc_39_typedef(void** state)
{
    (void)state;
    test_picoc_script("tests/39_typedef.c");
}

void test_picoc_40_stdio(void** state)
{
    (void)state;
    test_picoc_script("tests/40_stdio.c");
}

void test_picoc_41_hashif(void** state)
{
    (void)state;
    test_picoc_script("tests/41_hashif.c");
}

void test_picoc_43_void_param(void** state)
{
    (void)state;
    test_picoc_script("tests/43_void_param.c");
}

void test_picoc_44_scoped_declarations(void** state)
{
    (void)state;
    test_picoc_script("tests/44_scoped_declarations.c");
}

void test_picoc_45_empty_for(void** state)
{
    (void)state;
    test_picoc_script("tests/45_empty_for.c");
}

void test_picoc_47_switch_return(void** state)
{
    (void)state;
    test_picoc_script("tests/47_switch_return.c");
}

void test_picoc_48_nested_break(void** state)
{
    (void)state;
    test_picoc_script("tests/48_nested_break.c");
}

void test_picoc_49_bracket_evaluation(void** state)
{
    (void)state;
    test_picoc_script("tests/49_bracket_evaluation.c");
}

void test_picoc_50_logical_second_arg(void** state)
{
    (void)state;
    test_picoc_script("tests/50_logical_second_arg.c");
}

void test_picoc_51_static(void** state)
{
    (void)state;
    test_picoc_script("tests/51_static.c");
}

void test_picoc_52_unnamed_enum(void** state)
{
    (void)state;
    test_picoc_script("tests/52_unnamed_enum.c");
}

void test_picoc_54_goto(void** state)
{
    (void)state;
    test_picoc_script("tests/54_goto.c");
}

void test_picoc_55_array_initializer(void** state)
{
    (void)state;
    test_picoc_script("tests/55_array_initializer.c");
}

void test_picoc_56_cross_structure(void** state)
{
    (void)state;
    test_picoc_script("tests/56_cross_structure.c");
}

void test_picoc_57_macro_bug(void** state)
{
    (void)state;
    test_picoc_script("tests/57_macro_bug.c");
}

void test_picoc_58_return_outside(void** state)
{
    (void)state;
    test_picoc_script("tests/58_return_outside.c");
}

void test_picoc_59_break_before_loop(void** state)
{
    (void)state;
    test_picoc_script("tests/59_break_before_loop.c");
}

void test_picoc_60_local_vars(void** state)
{
    (void)state;
    test_picoc_script("tests/60_local_vars.c");
}

void test_picoc_61_initializers(void** state)
{
    (void)state;
    test_picoc_script("tests/61_initializers.c");
}

void test_picoc_62_float(void** state)
{
    (void)state;
    test_picoc_script("tests/62_float.c");
}

void test_picoc_63_typedef(void** state)
{
    (void)state;
    test_picoc_script("tests/63_typedef.c");
}

void test_picoc_64_double_prefix_op(void** state)
{
    (void)state;
    test_picoc_script("tests/64_double_prefix_op.c");
}

void test_picoc_65_typeless(void** state)
{
    (void)state;
    test_picoc_script("tests/65_typeless.c");
}

void test_picoc_66_printf_undefined(void** state)
{
    (void)state;
    test_picoc_script("tests/66_printf_undefined.c");
}

void test_picoc_67_macro_crash(void** state)
{
    (void)state;
    test_picoc_script("tests/67_macro_crash.c");
}

void test_picoc_68_return(void** state)
{
    (void)state;
    test_picoc_script("tests/68_return.c");
}

void test_picoc_rand0(void** state)
{
    (void)state;
    test_picoc_script("tests/rand0.c");
}

void test_picoc_rand1(void** state)
{
    (void)state;
    test_picoc_script("tests/rand1.c");
}

void test_picoc_rand2(void** state)
{
    (void)state;
    test_picoc_script("tests/rand2.c");
}

void test_picoc_rand3(void** state)
{
    (void)state;
    test_picoc_script("tests/rand3.c");
}

void test_picoc_rand4(void** state)
{
    (void)state;
    test_picoc_script("tests/rand4.c");
}

void test_picoc_rand5(void** state)
{
    (void)state;
    test_picoc_script("tests/rand5.c");
}

void test_picoc_rand6(void** state)
{
    (void)state;
    test_picoc_script("tests/rand6.c");
}

void test_picoc_rand7(void** state)
{
    (void)state;
    test_picoc_script("tests/rand7.c");
}

void test_picoc_rand8(void** state)
{
    (void)state;
    test_picoc_script("tests/rand8.c");
}

void test_picoc_rand9(void** state)
{
    (void)state;
    test_picoc_script("tests/rand9.c");
}

void test_picoc_rand10(void** state)
{
    (void)state;
    test_picoc_script("tests/rand10.c");
}

void test_picoc_rand11(void** state)
{
    (void)state;
    test_picoc_script("tests/rand11.c");
}

void test_picoc_rand12(void** state)
{
    (void)state;
    test_picoc_script("tests/rand12.c");
}

void test_picoc_rand13(void** state)
{
    (void)state;
    test_picoc_script("tests/rand13.c");
}

void test_picoc_rand14(void** state)
{
    (void)state;
    test_picoc_script("tests/rand14.c");
}

void test_picoc_rand15(void** state)
{
    (void)state;
    test_picoc_script("tests/rand15.c");
}

void test_picoc_rand16(void** state)
{
    (void)state;
    test_picoc_script("tests/rand16.c");
}

void test_picoc_rand17(void** state)
{
    (void)state;
    test_picoc_script("tests/rand17.c");
}

void test_picoc_rand18(void** state)
{
    (void)state;
    test_picoc_script("tests/rand18.c");
}

void test_picoc_rand19(void** state)
{
    (void)state;
    test_picoc_script("tests/rand19.c");
}

void test_picoc_rand20(void** state)
{
    (void)state;
    test_picoc_script("tests/rand20.c");
}

void test_picoc_rand21(void** state)
{
    (void)state;
    test_picoc_script("tests/rand21.c");
}

void test_picoc_rand22(void** state)
{
    (void)state;
    test_picoc_script("tests/rand22.c");
}

void test_picoc_rand23(void** state)
{
    (void)state;
    test_picoc_script("tests/rand23.c");
}

void test_picoc_rand24(void** state)
{
    (void)state;
    test_picoc_script("tests/rand24.c");
}

void test_picoc_rand25(void** state)
{
    (void)state;
    test_picoc_script("tests/rand25.c");
}

void test_picoc_rand26(void** state)
{
    (void)state;
    test_picoc_script("tests/rand26.c");
}

void test_picoc_rand27(void** state)
{
    (void)state;
    test_picoc_script("tests/rand27.c");
}

void test_picoc_rand28(void** state)
{
    (void)state;
    test_picoc_script("tests/rand28.c");
}

void test_picoc_rand29(void** state)
{
    (void)state;
    test_picoc_script("tests/rand29.c");
}

void test_picoc_rand30(void** state)
{
    (void)state;
    test_picoc_script("tests/rand30.c");
}

void test_picoc_rand31(void** state)
{
    (void)state;
    test_picoc_script("tests/rand31.c");
}

void test_picoc_rand32(void** state)
{
    (void)state;
    test_picoc_script("tests/rand32.c");
}

void test_picoc_rand33(void** state)
{
    (void)state;
    test_picoc_script("tests/rand33.c");
}

void test_picoc_rand34(void** state)
{
    (void)state;
    test_picoc_script("tests/rand34.c");
}

void test_picoc_rand35(void** state)
{
    (void)state;
    test_picoc_script("tests/rand35.c");
}

void test_picoc_rand36(void** state)
{
    (void)state;
    test_picoc_script("tests/rand36.c");
}

void test_picoc_rand37(void** state)
{
    (void)state;
    test_picoc_script("tests/rand37.c");
}

void test_picoc_rand38(void** state)
{
    (void)state;
    test_picoc_script("tests/rand38.c");
}

void test_picoc_rand39(void** state)
{
    (void)state;
    test_picoc_script("tests/rand39.c");
}

void test_picoc_rand40(void** state)
{
    (void)state;
    test_picoc_script("tests/rand40.c");
}

void test_picoc_rand41(void** state)
{
    (void)state;
    test_picoc_script("tests/rand41.c");
}

void test_picoc_rand42(void** state)
{
    (void)state;
    test_picoc_script("tests/rand42.c");
}

void test_picoc_rand43(void** state)
{
    (void)state;
    test_picoc_script("tests/rand43.c");
}

void test_picoc_rand44(void** state)
{
    (void)state;
    test_picoc_script("tests/rand44.c");
}

void test_picoc_rand45(void** state)
{
    (void)state;
    test_picoc_script("tests/rand45.c");
}

void test_picoc_rand46(void** state)
{
    (void)state;
    test_picoc_script("tests/rand46.c");
}

void test_picoc_rand47(void** state)
{
    (void)state;
    test_picoc_script("tests/rand47.c");
}

void test_picoc_rand48(void** state)
{
    (void)state;
    test_picoc_script("tests/rand48.c");
}

void test_picoc_rand49(void** state)
{
    (void)state;
    test_picoc_script("tests/rand49.c");
}

void test_picoc_rand50(void** state)
{
    (void)state;
    test_picoc_script("tests/rand50.c");
}

void test_picoc_rand51(void** state)
{
    (void)state;
    test_picoc_script("tests/rand51.c");
}

void test_picoc_rand52(void** state)
{
    (void)state;
    test_picoc_script("tests/rand52.c");
}

void test_picoc_rand53(void** state)
{
    (void)state;
    test_picoc_script("tests/rand53.c");
}

void test_picoc_rand54(void** state)
{
    (void)state;
    test_picoc_script("tests/rand54.c");
}

void test_picoc_rand55(void** state)
{
    (void)state;
    test_picoc_script("tests/rand55.c");
}

void test_picoc_rand56(void** state)
{
    (void)state;
    test_picoc_script("tests/rand56.c");
}

void test_picoc_rand57(void** state)
{
    (void)state;
    test_picoc_script("tests/rand57.c");
}

void test_picoc_rand58(void** state)
{
    (void)state;
    test_picoc_script("tests/rand58.c");
}

void test_picoc_rand59(void** state)
{
    (void)state;
    test_picoc_script("tests/rand59.c");
}

void test_picoc_rand60(void** state)
{
    (void)state;
    test_picoc_script("tests/rand60.c");
}

void test_picoc_rand61(void** state)
{
    (void)state;
    test_picoc_script("tests/rand61.c");
}

void test_picoc_rand62(void** state)
{
    (void)state;
    test_picoc_script("tests/rand62.c");
}

void test_picoc_rand63(void** state)
{
    (void)state;
    test_picoc_script("tests/rand63.c");
}

void test_picoc_rand64(void** state)
{
    (void)state;
    test_picoc_script("tests/rand64.c");
}

void test_picoc_rand65(void** state)
{
    (void)state;
    test_picoc_script("tests/rand65.c");
}

void test_picoc_rand66(void** state)
{
    (void)state;
    test_picoc_script("tests/rand66.c");
}

void test_picoc_rand67(void** state)
{
    (void)state;
    test_picoc_script("tests/rand67.c");
}

void test_picoc_rand68(void** state)
{
    (void)state;
    test_picoc_script("tests/rand68.c");
}

void test_picoc_rand69(void** state)
{
    (void)state;
    test_picoc_script("tests/rand69.c");
}

void test_picoc_rand70(void** state)
{
    (void)state;
    test_picoc_script("tests/rand70.c");
}

void test_picoc_rand71(void** state)
{
    (void)state;
    test_picoc_script("tests/rand71.c");
}

void test_picoc_rand72(void** state)
{
    (void)state;
    test_picoc_script("tests/rand72.c");
}

void test_picoc_rand73(void** state)
{
    (void)state;
    test_picoc_script("tests/rand73.c");
}

void test_picoc_rand74(void** state)
{
    (void)state;
    test_picoc_script("tests/rand74.c");
}

void test_picoc_rand75(void** state)
{
    (void)state;
    test_picoc_script("tests/rand75.c");
}

void test_picoc_rand76(void** state)
{
    (void)state;
    test_picoc_script("tests/rand76.c");
}

void test_picoc_rand77(void** state)
{
    (void)state;
    test_picoc_script("tests/rand77.c");
}

void test_picoc_rand78(void** state)
{
    (void)state;
    test_picoc_script("tests/rand78.c");
}

void test_picoc_rand79(void** state)
{
    (void)state;
    test_picoc_script("tests/rand79.c");
}

void test_picoc_rand80(void** state)
{
    (void)state;
    test_picoc_script("tests/rand80.c");
}

void test_picoc_rand81(void** state)
{
    (void)state;
    test_picoc_script("tests/rand81.c");
}

void test_picoc_rand82(void** state)
{
    (void)state;
    test_picoc_script("tests/rand82.c");
}

void test_picoc_rand83(void** state)
{
    (void)state;
    test_picoc_script("tests/rand83.c");
}

void test_picoc_rand84(void** state)
{
    (void)state;
    test_picoc_script("tests/rand84.c");
}

void test_picoc_rand85(void** state)
{
    (void)state;
    test_picoc_script("tests/rand85.c");
}

void test_picoc_rand86(void** state)
{
    (void)state;
    test_picoc_script("tests/rand86.c");
}

void test_picoc_rand87(void** state)
{
    (void)state;
    test_picoc_script("tests/rand87.c");
}

void test_picoc_rand88(void** state)
{
    (void)state;
    test_picoc_script("tests/rand88.c");
}

void test_picoc_rand89(void** state)
{
    (void)state;
    test_picoc_script("tests/rand89.c");
}

void test_picoc_rand90(void** state)
{
    (void)state;
    test_picoc_script("tests/rand90.c");
}

void test_picoc_rand91(void** state)
{
    (void)state;
    test_picoc_script("tests/rand91.c");
}

void test_picoc_rand92(void** state)
{
    (void)state;
    test_picoc_script("tests/rand92.c");
}

void test_picoc_rand93(void** state)
{
    (void)state;
    test_picoc_script("tests/rand93.c");
}

void test_picoc_rand94(void** state)
{
    (void)state;
    test_picoc_script("tests/rand94.c");
}

void test_picoc_rand95(void** state)
{
    (void)state;
    test_picoc_script("tests/rand95.c");
}

void test_picoc_rand96(void** state)
{
    (void)state;
    test_picoc_script("tests/rand96.c");
}

void test_picoc_rand97(void** state)
{
    (void)state;
    test_picoc_script("tests/rand97.c");
}

void test_picoc_rand98(void** state)
{
    (void)state;
    test_picoc_script("tests/rand98.c");
}

void test_picoc_rand99(void** state)
{
    (void)state;
    test_picoc_script("tests/rand99.c");
}

void test_picoc_rand100(void** state)
{
    (void)state;
    test_picoc_script("tests/rand100.c");
}

void test_picoc_rand101(void** state)
{
    (void)state;
    test_picoc_script("tests/rand101.c");
}

void test_picoc_rand102(void** state)
{
    (void)state;
    test_picoc_script("tests/rand102.c");
}

void test_picoc_rand103(void** state)
{
    (void)state;
    test_picoc_script("tests/rand103.c");
}

void test_picoc_rand104(void** state)
{
    (void)state;
    test_picoc_script("tests/rand104.c");
}

void test_picoc_rand105(void** state)
{
    (void)state;
    test_picoc_script("tests/rand105.c");
}

void test_picoc_rand106(void** state)
{
    (void)state;
    test_picoc_script("tests/rand106.c");
}

void test_picoc_rand107(void** state)
{
    (void)state;
    test_picoc_script("tests/rand107.c");
}

void test_picoc_rand108(void** state)
{
    (void)state;
    test_picoc_script("tests/rand108.c");
}

void test_picoc_rand109(void** state)
{
    (void)state;
    test_picoc_script("tests/rand109.c");
}

void test_picoc_rand110(void** state)
{
    (void)state;
    test_picoc_script("tests/rand110.c");
}

static void assert_files_equal(const char *file1_path, const char *file2_path)
{
    FILE *file1, *file2;
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

    // Check if both files reached EOF, otherwise they are of different sizes
    if (ch1 != ch2)
    {
        fclose(file1);
        fclose(file2);
        fail_msg("Files %s and %s have different sizes.", file1_path, file2_path);
    }

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

static void test_picoc_script(const char* script_path)
{
    core_t      core = { 0 };
    char        result_file_path[1024];
    char        expect_file_path[1024];
    char        base_name_no_ext[1024];
    const char* base_name;
    char*       dot;
    int         original_stdout_fd;
    size_t      path_len;

    base_name = strrchr(script_path, '/');
    if (!base_name)
    {
        base_name = strrchr(script_path, '\\');
    }
    if (base_name)
    {
        base_name++;
    }
    else
    {
        base_name = script_path;
    }

    path_len = base_name - script_path;
    strncpy(result_file_path, script_path, path_len);
    result_file_path[path_len] = '\0';
    strncpy(expect_file_path, script_path, path_len);
    expect_file_path[path_len] = '\0';

    strncpy(base_name_no_ext, base_name, sizeof(base_name_no_ext));
    dot = strrchr(base_name_no_ext, '.');
    if (dot)
    {
        *dot = '\0';
    }

    snprintf(result_file_path + path_len, sizeof(result_file_path) - path_len, "%s.result", base_name_no_ext);
    snprintf(expect_file_path + path_len, sizeof(expect_file_path) - path_len, "%s.expect", base_name_no_ext);

    scripts_init(&core);

    redirect_stdout_to_file(result_file_path);
    run_script(script_path, &core);
    restore_stdout();

    assert_files_equal(result_file_path, expect_file_path);
    remove(result_file_path);

    scripts_deinit(&core);
}
