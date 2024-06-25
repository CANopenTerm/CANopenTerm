/** @file unit_tests.c
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
#include <cmocka.h>
#include "can.h"
#include "core.h"
#include "nmt_client.h"
#include "os.h"

uint32 __wrap_can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    (void)message;
    (void)disp_mode;
    (void)comment;

    return 0;
}

static void test_nmt_send_command(void **state)
{
    assert_true(nmt_send_command(0x20, NMT_OPERATIONAL,     SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_STOP,            SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_PRE_OPERATIONAL, SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_RESET_NODE,      SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_RESET_COMM,      SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, 0x03,                SILENT, NULL) == NMT_UNKNOWN_COMMAND);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_nmt_send_command),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
