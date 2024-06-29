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
    assert_true(nmt_send_command(0x20, NMT_OPERATIONAL,     SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_STOP,            SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_PRE_OPERATIONAL, SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_RESET_NODE,      SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, NMT_RESET_COMM,      SILENT, NULL) == ALL_OK);
    assert_true(nmt_send_command(0x20, 0x03,                SILENT, NULL) == NMT_UNKNOWN_COMMAND);
}

void test_nmt_print_help(void** state)
{
    assert_true(nmt_print_help(SILENT) == NOTHING_TO_DO);
}
