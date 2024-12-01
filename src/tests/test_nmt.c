/** @file test_nmt.c
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
#include "nmt.h"
#include "os.h"
#include "test_nmt.h"

void test_nmt_send_command(void** state)
{
    int result;

    (void)state;

    result = nmt_send_command(0x20, NMT_OPERATIONAL, SILENT, NULL);
    assert_true(result == ALL_OK || result == CAN_WRITE_ERROR);

    result = nmt_send_command(0x20, NMT_STOP, SILENT, NULL);
    assert_true(result == ALL_OK || result == CAN_WRITE_ERROR);

    result = nmt_send_command(0x20, NMT_PRE_OPERATIONAL, SILENT, NULL);
    assert_true(result == ALL_OK || result == CAN_WRITE_ERROR);

    result = nmt_send_command(0x20, NMT_RESET_NODE, SILENT, NULL);
    assert_true(result == ALL_OK || result == CAN_WRITE_ERROR);

    result = nmt_send_command(0x20, NMT_RESET_COMM, SILENT, NULL);
    assert_true(result == ALL_OK || result == CAN_WRITE_ERROR);

    assert_true(nmt_send_command(0x20, 0x03, SILENT, NULL) == NMT_UNKNOWN_COMMAND);
}

void test_nmt_print_help(void** state)
{
    (void)state;

    assert_true(nmt_print_help(SILENT) == NOTHING_TO_DO);
}
