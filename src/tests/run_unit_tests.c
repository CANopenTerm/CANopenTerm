/** @file run_unit_tests.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cmocka.h"
#include "test_buffer.h"
#include "test_codb.h"
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
            cmocka_unit_test(test_codb2json),
            cmocka_unit_test(test_use_buffer),
            cmocka_unit_test(test_dict_lookup),
            cmocka_unit_test(test_dict_lookup_raw),
            cmocka_unit_test(test_has_valid_extension),
            cmocka_unit_test(test_lua),
            cmocka_unit_test(test_python_01_int),
            cmocka_unit_test(test_python_01_int),
            cmocka_unit_test(test_python_02_float),
            cmocka_unit_test(test_python_03_bool),
            cmocka_unit_test(test_python_04_line_continue),
            cmocka_unit_test(test_python_04_str),
            cmocka_unit_test(test_python_05_list),
            cmocka_unit_test(test_python_06_tuple),
            cmocka_unit_test(test_python_07_listcomp),
            cmocka_unit_test(test_python_08_dictcomp),
            cmocka_unit_test(test_python_08_dict),
            cmocka_unit_test(test_python_15_assign),
            cmocka_unit_test(test_python_15_cmp),
            cmocka_unit_test(test_python_15_controlflow),
            cmocka_unit_test(test_python_16_functions),
            cmocka_unit_test(test_python_16_typehints),
            cmocka_unit_test(test_python_24_inline_blocks),
            cmocka_unit_test(test_python_25_rfstring),
            cmocka_unit_test(test_python_26_multiline),
            cmocka_unit_test(test_python_28_exception),
            cmocka_unit_test(test_python_29_iter),
            /* cmocka_unit_test(test_python_30_import), */
            cmocka_unit_test(test_python_40_class),
            cmocka_unit_test(test_python_41_class_ex),
            cmocka_unit_test(test_python_42_decorator),
            cmocka_unit_test(test_python_43_closure),
            cmocka_unit_test(test_python_44_star),
            cmocka_unit_test(test_python_46_bytes),
            cmocka_unit_test(test_python_47_set),
            cmocka_unit_test(test_python_48_setcomp),
            cmocka_unit_test(test_python_50_reflection),
            cmocka_unit_test(test_python_51_yield),
            cmocka_unit_test(test_python_52_context),
            cmocka_unit_test(test_python_66_eval),
            cmocka_unit_test(test_python_70_bisect),
            cmocka_unit_test(test_python_70_builtins),
            cmocka_unit_test(test_python_70_heapq),
            cmocka_unit_test(test_python_70_math),
            cmocka_unit_test(test_python_70_random),
            cmocka_unit_test(test_python_71_cmath),
            cmocka_unit_test(test_python_71_gc),
            cmocka_unit_test(test_python_72_collections),
            cmocka_unit_test(test_python_72_json),
            cmocka_unit_test(test_python_73_functools),
            cmocka_unit_test(test_python_73_json_alt),
            cmocka_unit_test(test_python_73_typing),
            cmocka_unit_test(test_python_74_operator),
            cmocka_unit_test(test_python_75_compile),
            cmocka_unit_test(test_python_76_dna),
            cmocka_unit_test(test_python_76_misc),
            cmocka_unit_test(test_python_76_prime),
            cmocka_unit_test(test_python_77_builtin_func),
            cmocka_unit_test(test_python_79_datetime),
            cmocka_unit_test(test_python_79_easing),
            /* cmocka_unit_test(test_python_80_sys), */
            cmocka_unit_test(test_python_80_traceback),
            cmocka_unit_test(test_python_81_dataclasses),
            cmocka_unit_test(test_python_82_enum),
            cmocka_unit_test(test_python_90_array2d),
            /* cmocka_unit_test(test_python_90_pickle), */
            /* cmocka_unit_test(test_python_91_line_profiler), */
            cmocka_unit_test(test_python_91_pdb),
            cmocka_unit_test(test_python_95_bugs),
            cmocka_unit_test(test_python_95_dis),
            cmocka_unit_test(test_python_96_pep695_py312),
            cmocka_unit_test(test_python_99_extras),
            cmocka_unit_test(test_nmt_print_help),
            cmocka_unit_test(test_os_atof),
            cmocka_unit_test(test_os_atoi),
            cmocka_unit_test(test_os_calloc),
            cmocka_unit_test(test_os_clock),
            cmocka_unit_test(test_os_closedir),
            cmocka_unit_test(test_os_delay),
            cmocka_unit_test(test_os_fclose),
            cmocka_unit_test(test_os_fgets),
            cmocka_unit_test(test_os_fopen),
            cmocka_unit_test(test_os_fread),
            cmocka_unit_test(test_os_free),
            cmocka_unit_test(test_os_freopen),
            cmocka_unit_test(test_os_fseek),
            cmocka_unit_test(test_os_ftell),
            cmocka_unit_test(test_os_get_error),
            cmocka_unit_test(test_os_get_ticks),
            cmocka_unit_test(test_os_isdigit),
            cmocka_unit_test(test_os_isprint),
            cmocka_unit_test(test_os_isspace),
            cmocka_unit_test(test_os_isxdigit),
            cmocka_unit_test(test_os_itoa),
            cmocka_unit_test(test_os_memcpy),
            cmocka_unit_test(test_os_memmove),
            cmocka_unit_test(test_os_memset),
            cmocka_unit_test(test_os_opendir),
            cmocka_unit_test(test_os_readdir),
            cmocka_unit_test(test_os_realloc),
            cmocka_unit_test(test_os_rewind),
            cmocka_unit_test(test_os_snprintf),
            cmocka_unit_test(test_os_strchr),
            cmocka_unit_test(test_os_strcmp),
            cmocka_unit_test(test_os_strcspn),
            cmocka_unit_test(test_os_strdup),
            cmocka_unit_test(test_os_strlcat),
            cmocka_unit_test(test_os_strlcpy),
            cmocka_unit_test(test_os_strlen),
            cmocka_unit_test(test_os_strncmp),
            cmocka_unit_test(test_os_strrchr),
            cmocka_unit_test(test_os_strstr),
            cmocka_unit_test(test_os_strtokr),
            cmocka_unit_test(test_os_strtol),
            cmocka_unit_test(test_os_strtoul),
            cmocka_unit_test(test_os_strtoull),
            cmocka_unit_test(test_os_swap_be_32),
            cmocka_unit_test(test_os_tolower),
            cmocka_unit_test(test_os_toupper),
            cmocka_unit_test(test_os_vsnprintf),
            cmocka_unit_test(test_sdo_lookup_abort_code),
            cmocka_unit_test(test_uint8),
            cmocka_unit_test(test_uint16),
            cmocka_unit_test(test_uint32),
            cmocka_unit_test(test_uint64),
            cmocka_unit_test(test_variadic_functions),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
