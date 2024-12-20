/** @file test_nmt.c
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
#include "nmt.h"
#include "os.h"
#include "test_nmt.h"

void test_nmt_print_help(void** state)
{
    (void)state;

    assert_true(nmt_print_help(SILENT) == NOTHING_TO_DO);
}
