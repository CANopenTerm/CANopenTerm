/** @file python_test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "os.h"
#include "pocketpy.h"
#include "test_report.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_test_add_result(int argc, py_Ref argv);
bool py_test_clear_results(int argc, py_Ref argv);
bool py_test_generate_report(int argc, py_Ref argv);

void python_test_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    //py_bind(mod, "test_add_result(can_id, data_length, data=0, is_extended=False, show_output=False, comment=\"\")", py_can_write);

    py_bindfunc(mod, "test_clear_results",   py_test_clear_results);
    py_bindfunc(mod, "test_generate_report", py_test_generate_report);
}

bool py_test_add_result(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}

bool py_test_clear_results(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}

bool py_test_generate_report(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;
    return IS_TRUE;
}
