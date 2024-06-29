/** @file test_scripts.c
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
#include "os.h"
#include "scripts.h"
#include "test_scripts.h"

void test_has_lua_extension(void** state)
{
    (void)state;

    assert_true(has_lua_extension("exceptional_script.lua") == IS_TRUE);
    assert_true(has_lua_extension("mediocre_script.py")     == IS_FALSE);
}
