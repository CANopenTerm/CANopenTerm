/** @file run_unit_tests.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "core.h"
#include "cmocka.h"
#include "test_buffer.h"
#include "test_can.h"
#include "test_codb.h"
#include "test_dict.h"
#include "test_nmt.h"
#include "test_os.h"
#include "test_pdo.h"
#include "test_scripts.h"
#include "test_sdo.h"
#include "test_table.h"

core_t* core = NULL;

int main(void)
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(test_buffer_init),
            cmocka_unit_test(test_buffer_write),
            cmocka_unit_test(test_buffer_write_grow),
            cmocka_unit_test(test_codb2json),
            cmocka_unit_test(test_use_buffer),
            cmocka_unit_test(test_dict_lookup),
            cmocka_unit_test(test_dict_lookup_raw),
            cmocka_unit_test(test_has_valid_extension),
            cmocka_unit_test(test_lua),
            cmocka_unit_test(test_python_010_int),
            cmocka_unit_test(test_python_020_float),
            cmocka_unit_test(test_python_030_bool),
            cmocka_unit_test(test_python_040_line_continue),
            cmocka_unit_test(test_python_041_str),
            cmocka_unit_test(test_python_042_str_mod),
            cmocka_unit_test(test_python_050_list),
            cmocka_unit_test(test_python_060_tuple),
            cmocka_unit_test(test_python_070_listcomp),
            cmocka_unit_test(test_python_080_dict),
            cmocka_unit_test(test_python_081_dictcomp),
            cmocka_unit_test(test_python_150_assign),
            cmocka_unit_test(test_python_151_cmp),
            cmocka_unit_test(test_python_152_controlflow),
            cmocka_unit_test(test_python_160_functions),
            cmocka_unit_test(test_python_161_typehints),
            cmocka_unit_test(test_python_240_inline_blocks),
            cmocka_unit_test(test_python_250_rfstring),
            cmocka_unit_test(test_python_260_multiline),
            cmocka_unit_test(test_python_280_exception),
            cmocka_unit_test(test_python_290_iter),
            /* cmocka_unit_test(test_python_300_import), */
            /* cmocka_unit_test(test_python_301_import1), */
            /* cmocka_unit_test(test_python_310_modulereload), */
            cmocka_unit_test(test_python_400_class),
            cmocka_unit_test(test_python_410_class_ex),
            cmocka_unit_test(test_python_420_decorator),
            cmocka_unit_test(test_python_430_closure),
            cmocka_unit_test(test_python_440_star),
            cmocka_unit_test(test_python_460_bytes),
            cmocka_unit_test(test_python_470_set),
            cmocka_unit_test(test_python_480_setcomp),
            cmocka_unit_test(test_python_500_reflection),
            cmocka_unit_test(test_python_510_yield),
            cmocka_unit_test(test_python_520_context),
            cmocka_unit_test(test_python_660_eval),
            cmocka_unit_test(test_python_661_exec_bug),
            /* cmocka_unit_test(test_python_670_locals_vs_globals), */
            cmocka_unit_test(test_python_700_base64),
            cmocka_unit_test(test_python_701_bisect),
            cmocka_unit_test(test_python_702_builtins),
            cmocka_unit_test(test_python_703_heapq),
            cmocka_unit_test(test_python_704_math),
            cmocka_unit_test(test_python_705_random),
            cmocka_unit_test(test_python_710_cmath),
            cmocka_unit_test(test_python_711_gc),
            cmocka_unit_test(test_python_720_collections),
            cmocka_unit_test(test_python_721_json),
            /* cmocka_unit_test(test_python_722_lz4), */
            /* cmocka_unit_test(test_python_723_msgpack), */
            cmocka_unit_test(test_python_730_functools),
            cmocka_unit_test(test_python_731_json_alt),
            cmocka_unit_test(test_python_732_json_indent),
            /* cmocka_unit_test(test_python_733_typing), */
            cmocka_unit_test(test_python_740_operator),
            cmocka_unit_test(test_python_750_compile),
            cmocka_unit_test(test_python_760_dna),
            cmocka_unit_test(test_python_761_misc),
            cmocka_unit_test(test_python_762_prime),
            cmocka_unit_test(test_python_770_builtin_func_1),
            cmocka_unit_test(test_python_771_builtin_func_2),
            cmocka_unit_test(test_python_790_datetime),
            cmocka_unit_test(test_python_791_easing),
            cmocka_unit_test(test_python_792_file),
            cmocka_unit_test(test_python_793_stdc),
            cmocka_unit_test(test_python_800_color32),
            /* cmocka_unit_test(test_python_801_sys), */
            /* cmocka_unit_test(test_python_802_traceback), */
            cmocka_unit_test(test_python_803_vmath),
            cmocka_unit_test(test_python_810_dataclasses),
            cmocka_unit_test(test_python_820_enum),
            cmocka_unit_test(test_python_830_unicodedata),
            cmocka_unit_test(test_python_900_array2d),
            /* cmocka_unit_test(test_python_901_array2d_extra1), */
            cmocka_unit_test(test_python_902_chunked_array2d),
            /* cmocka_unit_test(test_python_903_colorcvt), */
            /* cmocka_unit_test(test_python_904_pickle), */
            /* cmocka_unit_test(test_python_910_line_profiler), */
            cmocka_unit_test(test_python_920_picoterm),
            cmocka_unit_test(test_python_921_pkpy),
            /* cmocka_unit_test(test_python_922_py_compile), */
            cmocka_unit_test(test_python_930_deterministic_float),
            cmocka_unit_test(test_python_950_bugs),
            cmocka_unit_test(test_python_951_dis),
            cmocka_unit_test(test_python_960_pep695_py312),
            cmocka_unit_test(test_python_970_inspect),
            cmocka_unit_test(test_python_980_thread),
            cmocka_unit_test(test_python_990_extras),
            cmocka_unit_test(test_nmt_print_help),
            cmocka_unit_test(test_nmt_send_command_invalid),
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
            cmocka_unit_test(test_os_strtokr_r),
            cmocka_unit_test(test_os_strtol),
            cmocka_unit_test(test_os_strtoul),
            cmocka_unit_test(test_os_strtoull),
            cmocka_unit_test(test_os_swap_be_32),
            cmocka_unit_test(test_os_tolower),
            cmocka_unit_test(test_os_toupper),
            cmocka_unit_test(test_os_vsnprintf),
            cmocka_unit_test(test_os_sscanf),
            cmocka_unit_test(test_os_swap_64),
            cmocka_unit_test(test_os_fix_path),
            cmocka_unit_test(test_os_get_user_directory),
            cmocka_unit_test(test_os_find_data_path),
            cmocka_unit_test(test_sdo_lookup_abort_code),
            cmocka_unit_test(test_uint8),
            cmocka_unit_test(test_uint16),
            cmocka_unit_test(test_uint32),
            cmocka_unit_test(test_uint64),
            cmocka_unit_test(test_variadic_functions),
            cmocka_unit_test(test_can_limit_node_id),
            cmocka_unit_test(test_can_is_can_initialised),
            cmocka_unit_test(test_pdo_is_id_valid),
            cmocka_unit_test(test_table_init),
            cmocka_unit_test(test_table_lifecycle),
            cmocka_unit_test(test_dict_lookup_unknown),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
