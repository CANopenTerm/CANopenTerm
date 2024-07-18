/** @file run_unit_tests.c
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
#include "cmocka.h"
#include "test_buffer.h"
#include "test_dict.h"
#include "test_nmt.h"
#include "test_os.h"
#include "test_scripts.h"
#include "test_sdo.h"

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_buffer_init),
        cmocka_unit_test(test_use_buffer),
        cmocka_unit_test(test_dict_lookup),
        cmocka_unit_test(test_has_valid_extension),
        cmocka_unit_test(test_lua),
        cmocka_unit_test(test_picoc_00_assignment),
        cmocka_unit_test(test_picoc_01_comment),
        cmocka_unit_test(test_picoc_02_printf),
        cmocka_unit_test(test_picoc_03_struct),
        cmocka_unit_test(test_picoc_04_for),
        cmocka_unit_test(test_picoc_05_array),
        cmocka_unit_test(test_picoc_06_case),
        cmocka_unit_test(test_picoc_07_function),
        cmocka_unit_test(test_picoc_08_while),
        cmocka_unit_test(test_picoc_09_do_while),
        cmocka_unit_test(test_picoc_10_pointer),
        cmocka_unit_test(test_picoc_11_precedence),
        cmocka_unit_test(test_picoc_12_hashdefine),
        cmocka_unit_test(test_picoc_13_integer_literals),
        cmocka_unit_test(test_picoc_14_if),
        cmocka_unit_test(test_picoc_15_recursion),
        cmocka_unit_test(test_picoc_16_nesting),
        cmocka_unit_test(test_picoc_17_enum),
        cmocka_unit_test(test_picoc_18_include),
        cmocka_unit_test(test_picoc_19_pointer_arithmetic),
        cmocka_unit_test(test_picoc_20_pointer_comparison),
        cmocka_unit_test(test_picoc_21_char_array),
        cmocka_unit_test(test_picoc_22_floating_point),
        cmocka_unit_test(test_picoc_23_type_coercion),
        cmocka_unit_test(test_picoc_24_math_library),
        cmocka_unit_test(test_picoc_25_quicksort),
        cmocka_unit_test(test_picoc_26_character_constants),
        cmocka_unit_test(test_picoc_27_sizeof),
        cmocka_unit_test(test_picoc_28_strings),
        cmocka_unit_test(test_picoc_29_array_address),
        cmocka_unit_test(test_picoc_30_hanoi),
        cmocka_unit_test(test_picoc_31_args),
        cmocka_unit_test(test_picoc_32_led),
        cmocka_unit_test(test_picoc_33_ternary_op),
        cmocka_unit_test(test_picoc_34_array_assignment),
        cmocka_unit_test(test_picoc_35_sizeof),
        cmocka_unit_test(test_picoc_36_array_initializers),
        cmocka_unit_test(test_picoc_37_sprintf),
        cmocka_unit_test(test_picoc_38_multiple_array_index),
        cmocka_unit_test(test_picoc_39_typedef),
        cmocka_unit_test(test_picoc_40_stdio),
        cmocka_unit_test(test_picoc_41_hashif),
        cmocka_unit_test(test_picoc_43_void_param),
        cmocka_unit_test(test_picoc_44_scoped_declarations),
        cmocka_unit_test(test_picoc_45_empty_for),
        cmocka_unit_test(test_picoc_47_switch_return),
        cmocka_unit_test(test_picoc_48_nested_break),
        cmocka_unit_test(test_picoc_49_bracket_evaluation),
        cmocka_unit_test(test_picoc_50_logical_second_arg),
        cmocka_unit_test(test_picoc_51_static),
        cmocka_unit_test(test_picoc_52_unnamed_enum),
        cmocka_unit_test(test_picoc_54_goto),
        cmocka_unit_test(test_picoc_55_array_initializer),
        cmocka_unit_test(test_picoc_56_cross_structure),
        cmocka_unit_test(test_picoc_57_macro_bug),
        cmocka_unit_test(test_picoc_58_return_outside),
        cmocka_unit_test(test_picoc_59_break_before_loop),
        cmocka_unit_test(test_picoc_60_local_vars),
        cmocka_unit_test(test_picoc_61_initializers),
        cmocka_unit_test(test_picoc_62_float),
        cmocka_unit_test(test_picoc_63_typedef),
        cmocka_unit_test(test_picoc_64_double_prefix_op),
        cmocka_unit_test(test_picoc_65_typeless),
        cmocka_unit_test(test_picoc_66_printf_undefined),
        cmocka_unit_test(test_picoc_67_macro_crash),
        cmocka_unit_test(test_picoc_68_return),


        cmocka_unit_test(test_nmt_send_command),
        cmocka_unit_test(test_nmt_print_help),
        cmocka_unit_test(test_os_calloc),
        cmocka_unit_test(test_os_free),
        cmocka_unit_test(test_os_isdigit),
        cmocka_unit_test(test_os_isprint),
        cmocka_unit_test(test_os_itoa),
        cmocka_unit_test(test_os_memcpy),
        cmocka_unit_test(test_os_memmove),
        cmocka_unit_test(test_os_memset),
        cmocka_unit_test(test_os_realloc),
        cmocka_unit_test(test_os_snprintf),
        cmocka_unit_test(test_os_strchr),
        cmocka_unit_test(test_os_strcmp),
        cmocka_unit_test(test_os_strcspn),
        cmocka_unit_test(test_os_strdup),
        cmocka_unit_test(test_os_strlcat),
        cmocka_unit_test(test_os_strlen),
        cmocka_unit_test(test_os_strncmp),
        cmocka_unit_test(test_os_strrchr),
        cmocka_unit_test(test_os_strtokr),
        cmocka_unit_test(test_os_strtol),
        cmocka_unit_test(test_os_strtoull),
        cmocka_unit_test(test_os_vsnprintf),
        cmocka_unit_test(test_os_swap_be_32),
        cmocka_unit_test(test_os_delay),
        cmocka_unit_test(test_os_get_error),
        cmocka_unit_test(test_os_get_ticks),
        cmocka_unit_test(test_sdo_lookup_abort_code),
        cmocka_unit_test(test_uint8),
        cmocka_unit_test(test_uint16),
        cmocka_unit_test(test_uint32),
        cmocka_unit_test(test_uint64),
        cmocka_unit_test(test_variadic_functions)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
