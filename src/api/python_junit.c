/** @file python_junit.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "junit.h"
#include "os.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_junit_add_result(int argc, py_Ref argv);
bool py_junit_clear_results(int argc, py_Ref argv);
bool py_junit_generate_report(int argc, py_Ref argv);

void python_junit_init(core_t* core)
{
    (void)core;
}

bool py_junit_add_result(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}

bool py_junit_clear_results(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}

bool py_junit_generate_report(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}
