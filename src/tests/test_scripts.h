/** @file test_scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_SCRIPTS_H
#define TEST_SCRIPTS_H

void test_has_valid_extension(void** state);
void test_lua(void** state);
void test_picoc_00_assignment(void** state);
void test_picoc_01_comment(void** state);
void test_picoc_02_printf(void** state);
void test_picoc_03_struct(void** state);
void test_picoc_04_for(void** state);
void test_picoc_05_array(void** state);
void test_picoc_06_case(void** state);
void test_picoc_07_function(void** state);
void test_picoc_08_while(void** state);
void test_picoc_09_do_while(void** state);
void test_picoc_10_pointer(void** state);
void test_picoc_11_precedence(void** state);
void test_picoc_12_hashdefine(void** state);
void test_picoc_13_integer_literals(void** state);
void test_picoc_14_if(void** state);
void test_picoc_15_recursion(void** state);
void test_picoc_16_nesting(void** state);
void test_picoc_17_enum(void** state);
void test_picoc_18_include(void** state);
void test_picoc_19_pointer_arithmetic(void** state);
void test_picoc_20_pointer_comparison(void** state);
void test_picoc_21_char_array(void** state);
void test_picoc_22_floating_point(void** state);
void test_picoc_23_type_coercion(void** state);
void test_picoc_24_math_library(void** state);
void test_picoc_25_quicksort(void** state);
void test_picoc_26_character_constants(void** state);
void test_picoc_27_sizeof(void** state);
void test_picoc_28_strings(void** state);
void test_picoc_29_array_address(void** state);
void test_picoc_30_hanoi(void** state);
void test_picoc_31_args(void** state);
void test_picoc_32_led(void** state);
void test_picoc_33_ternary_op(void** state);
void test_picoc_34_array_assignment(void** state);
void test_picoc_35_sizeof(void** state);
void test_picoc_36_array_initializers(void** state);
void test_picoc_37_sprintf(void** state);
void test_picoc_38_multiple_array_index(void** state);
void test_picoc_39_typedef(void** state);
void test_picoc_40_stdio(void** state);
void test_picoc_41_hashif(void** state);
void test_picoc_43_void_param(void** state);
void test_picoc_44_scoped_declarations(void** state);
void test_picoc_45_empty_for(void** state);
void test_picoc_47_switch_return(void** state);
void test_picoc_48_nested_break(void** state);
void test_picoc_49_bracket_evaluation(void** state);
void test_picoc_50_logical_second_arg(void** state);
void test_picoc_51_static(void** state);
void test_picoc_52_unnamed_enum(void** state);
void test_picoc_54_goto(void** state);
void test_picoc_55_array_initializer(void** state);
void test_picoc_56_cross_structure(void** state);
void test_picoc_57_macro_bug(void** state);
void test_picoc_58_return_outside(void** state);
void test_picoc_59_break_before_loop(void** state);
void test_picoc_60_local_vars(void** state);
void test_picoc_61_initializers(void** state);
void test_picoc_62_float(void** state);
void test_picoc_63_typedef(void** state);
void test_picoc_64_double_prefix_op(void** state);
void test_picoc_65_typeless(void** state);
void test_picoc_66_printf_undefined(void** state);
void test_picoc_67_macro_crash(void** state);
void test_picoc_68_return(void** state);

#endif /* TEST_SCRIPTS_H */
