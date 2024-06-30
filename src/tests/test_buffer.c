/** @file test_buffer.c
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
#include "buffer.h"
#include "os.h"
#include "test_buffer.h"

void test_buffer_init(void** state)
{
    (void)state;

    assert_true(buffer_init(1024) == ALL_OK);
    buffer_free();
}

void test_use_buffer(void** state)
{
    (void)state;

    assert_true(use_buffer() == IS_FALSE);
    buffer_init(1024);
    assert_true(use_buffer() == IS_TRUE);
    buffer_free();
    assert_true(use_buffer() == IS_FALSE);
}
