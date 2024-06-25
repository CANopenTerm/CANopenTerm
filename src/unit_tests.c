/** @file unit_tests.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include "can.h"
#include "core.h"
#include "nmt_client.h"

Uint32 __wrap_can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    (void)message;
    (void)disp_mode;
    (void)comment;

    return 0;
}

static void test_nmt_send_command(void **state)
{
    assert_true(nmt_send_command(0x20, NMT_OPERATIONAL,     NO_OUTPUT, NULL) == 0);
    assert_true(nmt_send_command(0x20, NMT_STOP,            NO_OUTPUT, NULL) == 0);
    assert_true(nmt_send_command(0x20, NMT_PRE_OPERATIONAL, NO_OUTPUT, NULL) == 0);
    assert_true(nmt_send_command(0x20, NMT_RESET_NODE,      NO_OUTPUT, NULL) == 0);
    assert_true(nmt_send_command(0x20, NMT_RESET_COMM,      NO_OUTPUT, NULL) == 0);
    assert_true(nmt_send_command(0x20, 0x03,                NO_OUTPUT, NULL) == 0);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_nmt_send_command),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
