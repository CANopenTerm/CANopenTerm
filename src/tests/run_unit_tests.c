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
#include "test_dict.h"
#include "test_nmt_client.h"

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_dict_lookup),
        cmocka_unit_test(test_nmt_send_command),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
