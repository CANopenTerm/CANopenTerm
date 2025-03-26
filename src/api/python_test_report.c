/** @file python_test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "eds.h"
#include "os.h"
#include "pocketpy.h"
#include "test_report.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_test_add_result(int argc, py_Ref argv);
bool py_test_eds_file(int argc, py_Ref argv);
bool py_test_clear_results(int argc, py_Ref argv);
bool py_test_generate_report(int argc, py_Ref argv);

void python_test_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "test_add_result(has_passed=False, time=0.0, package=\"Tests\", class_name=\"Generic\", test_name=\"UnnamedTest\", error_type=\"AssertionError\", error_message=\"No error message provided\", call_stack=\"<!-- No call stack provided. -->\")", py_test_add_result);
    py_bind(mod, "test_eds_file(node_id, file_name, package=\"EDS\")", py_test_eds_file);
    py_bind(mod, "test_generate_report(file_name=\"test_report.xml\")", py_test_generate_report);

    py_bindfunc(mod, "test_clear_results", py_test_clear_results);
}

bool py_test_add_result(int argc, py_Ref argv)
{
    test_result_t result = {0};

    PY_CHECK_ARGC(8);
    PY_CHECK_ARG_TYPE(0, tp_bool);
    PY_CHECK_ARG_TYPE(1, tp_float);
    PY_CHECK_ARG_TYPE(2, tp_str);
    PY_CHECK_ARG_TYPE(3, tp_str);
    PY_CHECK_ARG_TYPE(4, tp_str);
    PY_CHECK_ARG_TYPE(5, tp_str);
    PY_CHECK_ARG_TYPE(6, tp_str);
    PY_CHECK_ARG_TYPE(7, tp_str);

    result.has_passed    = py_tobool(py_arg(0));
    result.time          = py_tofloat(py_arg(1));
    result.package       = py_tostr(py_arg(2));
    result.class_name    = py_tostr(py_arg(3));
    result.test_name     = py_tostr(py_arg(4));
    result.error_type    = py_tostr(py_arg(5));
    result.error_message = py_tostr(py_arg(6));
    result.call_stack    = py_tostr(py_arg(7));

    test_add_result(&result);
    py_newnone(py_retval());

    return true;
}

bool py_test_eds_file(int argc, py_Ref argv)
{
    int         node_id;
    const char* file_name;
    const char* package;

    (void)argc;
    (void)argv;

    PY_CHECK_ARGC(3);

    node_id   = py_toint(py_arg(0));
    file_name = py_tostr(py_arg(1));
    package   = py_tostr(py_arg(2));

    run_conformance_test(file_name, package, node_id, SCRIPT_MODE);

    py_newnone(py_retval());

    return true;
}

bool py_test_clear_results(int argc, py_Ref argv)
{
    (void)argc;
    (void)argv;

    PY_CHECK_ARGC(0);

    test_clear_results();
    py_newnone(py_retval());

    return true;
}

bool py_test_generate_report(int argc, py_Ref argv)
{
    const char* file_name;

    (void)argc;
    (void)argv;

    PY_CHECK_ARGC(1);

    file_name = py_tostr(py_arg(0));

    py_newnone(py_retval());

    if (ALL_OK == test_generate_report(file_name))
    {
        return true;
    }
    else
    {
        return false;
    }
}
