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
        cmocka_unit_test(test_has_valid_extension),
        cmocka_unit_test(test_lua),
        //cmocka_unit_test(test_nmt_send_command),
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
