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
    test_picoc_script("00_assignment");
}

void test_picoc_00_linked_list(void** state)
{
    (void)state;
    test_picoc_script("00_linked_list");
}

void test_picoc_01_comment(void** state)
{
    (void)state;
    test_picoc_script("01_comment");
}

void test_picoc_02_printf(void** state)
{
    (void)state;
    test_picoc_script("02_printf");
}

void test_picoc_03_struct(void** state)
{
    (void)state;
    test_picoc_script("03_struct");
}

void test_picoc_04_for(void** state)
{
    (void)state;
    test_picoc_script("04_for");
}

void test_picoc_05_array(void** state)
{
    (void)state;
    test_picoc_script("05_array");
}

void test_picoc_06_case(void** state)
{
    (void)state;
    test_picoc_script("06_case");
}

void test_picoc_07_function(void** state)
{
    (void)state;
    test_picoc_script("07_function");
}

void test_picoc_08_while(void** state)
{
    (void)state;
    test_picoc_script("08_while");
}

void test_picoc_09_do_while(void** state)
{
    (void)state;
    test_picoc_script("09_do_while");
}

void test_picoc_10_pointer(void** state)
{
    (void)state;
    test_picoc_script("10_pointer");
}

void test_picoc_11_precedence(void** state)
{
    (void)state;
    test_picoc_script("11_precedence");
}

void test_picoc_12_hashdefine(void** state)
{
    (void)state;
    test_picoc_script("12_hashdefine");
}

void test_picoc_13_integer_literals(void** state)
{
    (void)state;
    test_picoc_script("13_integer_literals");
}

void test_picoc_14_if(void** state)
{
    (void)state;
    test_picoc_script("14_if");
}

void test_picoc_15_recursion(void** state)
{
    (void)state;
    test_picoc_script("15_recursion");
}

void test_picoc_16_nesting(void** state)
{
    (void)state;
    test_picoc_script("16_nesting");
}

void test_picoc_17_enum(void** state)
{
    (void)state;
    test_picoc_script("17_enum");
}

void test_picoc_18_include(void** state)
{
    (void)state;
    test_picoc_script("18_include");
}

void test_picoc_19_pointer_arithmetic(void** state)
{
    (void)state;
    test_picoc_script("19_pointer_arithmetic");
}

void test_picoc_20_pointer_comparison(void** state)
{
    (void)state;
    test_picoc_script("20_pointer_comparison");
}

void test_picoc_21_char_array(void** state)
{
    (void)state;
    test_picoc_script("21_char_array");
}

void test_picoc_22_floating_point(void** state)
{
    (void)state;
    test_picoc_script("22_floating_point");
}

void test_picoc_23_type_coercion(void** state)
{
    (void)state;
    test_picoc_script("23_type_coercion");
}

void test_picoc_24_math_library(void** state)
{
    (void)state;
    test_picoc_script("24_math_library");
}

void test_picoc_25_quicksort(void** state)
{
    (void)state;
    test_picoc_script("25_quicksort");
}

void test_picoc_26_character_constants(void** state)
{
    (void)state;
    test_picoc_script("26_character_constants");
}

void test_picoc_27_sizeof(void** state)
{
    (void)state;
    test_picoc_script("27_sizeof");
}

void test_picoc_28_strings(void** state)
{
    (void)state;
    test_picoc_script("28_strings");
}

void test_picoc_29_array_address(void** state)
{
    (void)state;
    test_picoc_script("29_array_address");
}

void test_picoc_30_hanoi(void** state)
{
    (void)state;
    test_picoc_script("30_hanoi");
}

void test_picoc_31_args(void** state)
{
    (void)state;
    test_picoc_script("31_args");
}

void test_picoc_32_led(void** state)
{
    (void)state;
    test_picoc_script("32_led");
}

void test_picoc_33_ternary_op(void** state)
{
    (void)state;
    test_picoc_script("33_ternary_op");
}

void test_picoc_34_array_assignment(void** state)
{
    (void)state;
    test_picoc_script("34_array_assignment");
}

void test_picoc_35_sizeof(void** state)
{
    (void)state;
    test_picoc_script("35_sizeof");
}

void test_picoc_36_array_initializers(void** state)
{
    (void)state;
    test_picoc_script("36_array_initializers");
}

void test_picoc_37_sprintf(void** state)
{
    (void)state;
    test_picoc_script("37_sprintf");
}

void test_picoc_38_multiple_array_index(void** state)
{
    (void)state;
    test_picoc_script("38_multiple_array_index");
}

void test_picoc_39_typedef(void** state)
{
    (void)state;
    test_picoc_script("39_typedef");
}

void test_picoc_40_stdio(void** state)
{
    (void)state;
    test_picoc_script("40_stdio");
}

void test_picoc_41_hashif(void** state)
{
    (void)state;
    test_picoc_script("41_hashif");
}

void test_picoc_43_void_param(void** state)
{
    (void)state;
    test_picoc_script("43_void_param");
}

void test_picoc_44_scoped_declarations(void** state)
{
    (void)state;
    test_picoc_script("44_scoped_declarations");
}

void test_picoc_45_empty_for(void** state)
{
    (void)state;
    test_picoc_script("45_empty_for");
}

void test_picoc_47_switch_return(void** state)
{
    (void)state;
    test_picoc_script("47_switch_return");
}

void test_picoc_48_nested_break(void** state)
{
    (void)state;
    test_picoc_script("48_nested_break");
}

void test_picoc_49_bracket_evaluation(void** state)
{
    (void)state;
    test_picoc_script("49_bracket_evaluation");
}

void test_picoc_50_logical_second_arg(void** state)
{
    (void)state;
    test_picoc_script("50_logical_second_arg");
}

void test_picoc_51_static(void** state)
{
    (void)state;
    test_picoc_script("51_static");
}

void test_picoc_52_unnamed_enum(void** state)
{
    (void)state;
    test_picoc_script("52_unnamed_enum");
}

void test_picoc_54_goto(void** state)
{
    (void)state;
    test_picoc_script("54_goto");
}

void test_picoc_55_array_initializer(void** state)
{
    (void)state;
    test_picoc_script("55_array_initializer");
}

void test_picoc_56_cross_structure(void** state)
{
    (void)state;
    test_picoc_script("56_cross_structure");
}

void test_picoc_57_macro_bug(void** state)
{
    (void)state;
    test_picoc_script("57_macro_bug");
}

void test_picoc_58_return_outside(void** state)
{
    (void)state;
    test_picoc_script("58_return_outside");
}

void test_picoc_59_break_before_loop(void** state)
{
    (void)state;
    test_picoc_script("59_break_before_loop");
}

void test_picoc_60_local_vars(void** state)
{
    (void)state;
    test_picoc_script("60_local_vars");
}

void test_picoc_61_initializers(void** state)
{
    (void)state;
    test_picoc_script("61_initializers");
}

void test_picoc_62_float(void** state)
{
    (void)state;
    test_picoc_script("62_float");
}

void test_picoc_63_typedef(void** state)
{
    (void)state;
    test_picoc_script("63_typedef");
}

void test_picoc_64_double_prefix_op(void** state)
{
    (void)state;
    test_picoc_script("64_double_prefix_op");
}

void test_picoc_65_typeless(void** state)
{
    (void)state;
    test_picoc_script("65_typeless");
}

void test_picoc_66_printf_undefined(void** state)
{
    (void)state;
    test_picoc_script("66_printf_undefined");
}

void test_picoc_67_macro_crash(void** state)
{
    (void)state;
    test_picoc_script("67_macro_crash");
}

void test_picoc_68_return(void** state)
{
    (void)state;
    test_picoc_script("68_return");
}

void test_picoc_rand0(void** state)
{
    (void)state;
    test_picoc_script("rand0");
}

void test_picoc_rand1(void** state)
{
    (void)state;
    test_picoc_script("rand1");
}

void test_picoc_rand2(void** state)
{
    (void)state;
    test_picoc_script("rand2");
}

void test_picoc_rand3(void** state)
{
    (void)state;
    test_picoc_script("rand3");
}

void test_picoc_rand4(void** state)
{
    (void)state;
    test_picoc_script("rand4");
}

void test_picoc_rand5(void** state)
{
    (void)state;
    test_picoc_script("rand5");
}

void test_picoc_rand6(void** state)
{
    (void)state;
    test_picoc_script("rand6");
}

void test_picoc_rand7(void** state)
{
    (void)state;
    test_picoc_script("rand7");
}

void test_picoc_rand8(void** state)
{
    (void)state;
    test_picoc_script("rand8");
}

void test_picoc_rand9(void** state)
{
    (void)state;
    test_picoc_script("rand9");
}

void test_picoc_rand10(void** state)
{
    (void)state;
    test_picoc_script("rand10");
}

void test_picoc_rand11(void** state)
{
    (void)state;
    test_picoc_script("rand11");
}

void test_picoc_rand12(void** state)
{
    (void)state;
    test_picoc_script("rand12");
}

void test_picoc_rand13(void** state)
{
    (void)state;
    test_picoc_script("rand13");
}

void test_picoc_rand14(void** state)
{
    (void)state;
    test_picoc_script("rand14");
}

void test_picoc_rand15(void** state)
{
    (void)state;
    test_picoc_script("rand15");
}

void test_picoc_rand16(void** state)
{
    (void)state;
    test_picoc_script("rand16");
}

void test_picoc_rand17(void** state)
{
    (void)state;
    test_picoc_script("rand17");
}

void test_picoc_rand18(void** state)
{
    (void)state;
    test_picoc_script("rand18");
}

void test_picoc_rand19(void** state)
{
    (void)state;
    test_picoc_script("rand19");
}

void test_picoc_rand20(void** state)
{
    (void)state;
    test_picoc_script("rand20");
}

void test_picoc_rand21(void** state)
{
    (void)state;
    test_picoc_script("rand21");
}

void test_picoc_rand22(void** state)
{
    (void)state;
    test_picoc_script("rand22");
}

void test_picoc_rand23(void** state)
{
    (void)state;
    test_picoc_script("rand23");
}

void test_picoc_rand24(void** state)
{
    (void)state;
    test_picoc_script("rand24");
}

void test_picoc_rand25(void** state)
{
    (void)state;
    test_picoc_script("rand25");
}

void test_picoc_rand26(void** state)
{
    (void)state;
    test_picoc_script("rand26");
}

void test_picoc_rand27(void** state)
{
    (void)state;
    test_picoc_script("rand27");
}

void test_picoc_rand28(void** state)
{
    (void)state;
    test_picoc_script("rand28");
}

void test_picoc_rand29(void** state)
{
    (void)state;
    test_picoc_script("rand29");
}

void test_picoc_rand30(void** state)
{
    (void)state;
    test_picoc_script("rand30");
}

void test_picoc_rand31(void** state)
{
    (void)state;
    test_picoc_script("rand31");
}

void test_picoc_rand32(void** state)
{
    (void)state;
    test_picoc_script("rand32");
}

void test_picoc_rand33(void** state)
{
    (void)state;
    test_picoc_script("rand33");
}

void test_picoc_rand34(void** state)
{
    (void)state;
    test_picoc_script("rand34");
}

void test_picoc_rand35(void** state)
{
    (void)state;
    test_picoc_script("rand35");
}

void test_picoc_rand36(void** state)
{
    (void)state;
    test_picoc_script("rand36");
}

void test_picoc_rand37(void** state)
{
    (void)state;
    test_picoc_script("rand37");
}

void test_picoc_rand38(void** state)
{
    (void)state;
    test_picoc_script("rand38");
}

void test_picoc_rand39(void** state)
{
    (void)state;
    test_picoc_script("rand39");
}

void test_picoc_rand40(void** state)
{
    (void)state;
    test_picoc_script("rand40");
}

void test_picoc_rand41(void** state)
{
    (void)state;
    test_picoc_script("rand41");
}

void test_picoc_rand42(void** state)
{
    (void)state;
    test_picoc_script("rand42");
}

void test_picoc_rand43(void** state)
{
    (void)state;
    test_picoc_script("rand43");
}

void test_picoc_rand44(void** state)
{
    (void)state;
    test_picoc_script("rand44");
}

void test_picoc_rand45(void** state)
{
    (void)state;
    test_picoc_script("rand45");
}

void test_picoc_rand46(void** state)
{
    (void)state;
    test_picoc_script("rand46");
}

void test_picoc_rand47(void** state)
{
    (void)state;
    test_picoc_script("rand47");
}

void test_picoc_rand48(void** state)
{
    (void)state;
    test_picoc_script("rand48");
}

void test_picoc_rand49(void** state)
{
    (void)state;
    test_picoc_script("rand49");
}

void test_picoc_rand50(void** state)
{
    (void)state;
    test_picoc_script("rand50");
}

void test_picoc_rand51(void** state)
{
    (void)state;
    test_picoc_script("rand51");
}

void test_picoc_rand52(void** state)
{
    (void)state;
    test_picoc_script("rand52");
}

void test_picoc_rand53(void** state)
{
    (void)state;
    test_picoc_script("rand53");
}

void test_picoc_rand54(void** state)
{
    (void)state;
    test_picoc_script("rand54");
}

void test_picoc_rand55(void** state)
{
    (void)state;
    test_picoc_script("rand55");
}

void test_picoc_rand56(void** state)
{
    (void)state;
    test_picoc_script("rand56");
}

void test_picoc_rand57(void** state)
{
    (void)state;
    test_picoc_script("rand57");
}

void test_picoc_rand58(void** state)
{
    (void)state;
    test_picoc_script("rand58");
}

void test_picoc_rand59(void** state)
{
    (void)state;
    test_picoc_script("rand59");
}

void test_picoc_rand60(void** state)
{
    (void)state;
    test_picoc_script("rand60");
}

void test_picoc_rand61(void** state)
{
    (void)state;
    test_picoc_script("rand61");
}

void test_picoc_rand62(void** state)
{
    (void)state;
    test_picoc_script("rand62");
}

void test_picoc_rand63(void** state)
{
    (void)state;
    test_picoc_script("rand63");
}

void test_picoc_rand64(void** state)
{
    (void)state;
    test_picoc_script("rand64");
}

void test_picoc_rand65(void** state)
{
    (void)state;
    test_picoc_script("rand65");
}

void test_picoc_rand66(void** state)
{
    (void)state;
    test_picoc_script("rand66");
}

void test_picoc_rand67(void** state)
{
    (void)state;
    test_picoc_script("rand67");
}

void test_picoc_rand68(void** state)
{
    (void)state;
    test_picoc_script("rand68");
}

void test_picoc_rand69(void** state)
{
    (void)state;
    test_picoc_script("rand69");
}

void test_picoc_rand70(void** state)
{
    (void)state;
    test_picoc_script("rand70");
}

void test_picoc_rand71(void** state)
{
    (void)state;
    test_picoc_script("rand71");
}

void test_picoc_rand72(void** state)
{
    (void)state;
    test_picoc_script("rand72");
}

void test_picoc_rand73(void** state)
{
    (void)state;
    test_picoc_script("rand73");
}

void test_picoc_rand74(void** state)
{
    (void)state;
    test_picoc_script("rand74");
}

void test_picoc_rand75(void** state)
{
    (void)state;
    test_picoc_script("rand75");
}

void test_picoc_rand76(void** state)
{
    (void)state;
    test_picoc_script("rand76");
}

void test_picoc_rand77(void** state)
{
    (void)state;
    test_picoc_script("rand77");
}

void test_picoc_rand78(void** state)
{
    (void)state;
    test_picoc_script("rand78");
}

void test_picoc_rand79(void** state)
{
    (void)state;
    test_picoc_script("rand79");
}

void test_picoc_rand80(void** state)
{
    (void)state;
    test_picoc_script("rand80");
}

void test_picoc_rand81(void** state)
{
    (void)state;
    test_picoc_script("rand81");
}

void test_picoc_rand82(void** state)
{
    (void)state;
    test_picoc_script("rand82");
}

void test_picoc_rand83(void** state)
{
    (void)state;
    test_picoc_script("rand83");
}

void test_picoc_rand84(void** state)
{
    (void)state;
    test_picoc_script("rand84");
}

void test_picoc_rand85(void** state)
{
    (void)state;
    test_picoc_script("rand85");
}

void test_picoc_rand86(void** state)
{
    (void)state;
    test_picoc_script("rand86");
}

void test_picoc_rand87(void** state)
{
    (void)state;
    test_picoc_script("rand87");
}

void test_picoc_rand88(void** state)
{
    (void)state;
    test_picoc_script("rand88");
}

void test_picoc_rand89(void** state)
{
    (void)state;
    test_picoc_script("rand89");
}

void test_picoc_rand90(void** state)
{
    (void)state;
    test_picoc_script("rand90");
}

void test_picoc_rand91(void** state)
{
    (void)state;
    test_picoc_script("rand91");
}

void test_picoc_rand92(void** state)
{
    (void)state;
    test_picoc_script("rand92");
}

void test_picoc_rand93(void** state)
{
    (void)state;
    test_picoc_script("rand93");
}

void test_picoc_rand94(void** state)
{
    (void)state;
    test_picoc_script("rand94");
}

void test_picoc_rand95(void** state)
{
    (void)state;
    test_picoc_script("rand95");
}

void test_picoc_rand96(void** state)
{
    (void)state;
    test_picoc_script("rand96");
}

void test_picoc_rand97(void** state)
{
    (void)state;
    test_picoc_script("rand97");
}

void test_picoc_rand98(void** state)
{
    (void)state;
    test_picoc_script("rand98");
}

void test_picoc_rand99(void** state)
{
    (void)state;
    test_picoc_script("rand99");
}

void test_picoc_rand100(void** state)
{
    (void)state;
    test_picoc_script("rand100");
}

void test_picoc_rand101(void** state)
{
    (void)state;
    test_picoc_script("rand101");
}

void test_picoc_rand102(void** state)
{
    (void)state;
    test_picoc_script("rand102");
}

void test_picoc_rand103(void** state)
{
    (void)state;
    test_picoc_script("rand103");
}

void test_picoc_rand104(void** state)
{
    (void)state;
    test_picoc_script("rand104");
}

void test_picoc_rand105(void** state)
{
    (void)state;
    test_picoc_script("rand105");
}

void test_picoc_rand106(void** state)
{
    (void)state;
    test_picoc_script("rand106");
}

void test_picoc_rand107(void** state)
{
    (void)state;
    test_picoc_script("rand107");
}

void test_picoc_rand108(void** state)
{
    (void)state;
    test_picoc_script("rand108");
}

void test_picoc_rand109(void** state)
{
    (void)state;
    test_picoc_script("rand109");
}

void test_picoc_rand110(void** state)
{
    (void)state;
    test_picoc_script("rand110");
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
    restore_stdout();

    assert_files_equal(result_file_path, expect_file_path);
    remove(result_file_path);

    scripts_deinit(&core);
}
