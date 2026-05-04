/** @file test_buffer.c
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

#include "buffer.h"
#include "cmocka.h"
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

    assert_true(use_buffer() == false);
    buffer_init(1024);
    assert_true(use_buffer() == true);
    buffer_free();
    assert_true(use_buffer() == false);
}

void test_buffer_write(void** state)
{
    (void)state;

    assert_true(buffer_init(64) == ALL_OK);
    assert_true(use_buffer() == true);

    buffer_write("Hello");
    buffer_write(", %s!", "world");
    buffer_flush();

    assert_true(use_buffer() == true);
    buffer_free();
    assert_true(use_buffer() == false);
}

void test_buffer_write_grow(void** state)
{
    int i;

    (void)state;

    assert_true(buffer_init(8) == ALL_OK);

    for (i = 0; i < 20; i += 1)
    {
        buffer_write("chunk%d ", i);
    }

    assert_true(use_buffer() == true);
    buffer_flush();
    buffer_free();
}
