/** @file python_pdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "os.h"
#include "pdo.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

extern void pdo_print_result(uint16 can_id, uint32 event_time_ms, uint64 data, bool_t was_successful, const char* comment);

bool py_pdo_add(int argc, py_Ref argv);
bool py_pdo_del(int argc, py_Ref argv);

void python_pdo_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "pdo_add(can_id, event_time_ms, length, data=0, show_output=False, comment=\"\")", py_pdo_add);
    py_bind(mod, "pdo_del(can_id, show_output=False, comment=\"\")", py_pdo_del);
}

bool py_pdo_add(int argc, py_Ref argv)
{
    int         can_id;
    int         event_time_ms;
    int         length;
    uint64      data;
    bool_t      show_output;
    const char* comment;
    bool_t      was_successful;
    disp_mode_t disp_mode = SILENT;

    PY_CHECK_ARGC(6);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_bool);
    PY_CHECK_ARG_TYPE(5, tp_str);

    can_id        = py_toint(py_arg(0));
    event_time_ms = py_toint(py_arg(1));
    length        = py_toint(py_arg(2));
    data          = py_toint(py_arg(3));
    show_output   = py_tobool(py_arg(4));
    comment       = py_tostr(py_arg(5));

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    was_successful = pdo_add(can_id, event_time_ms, length, data, disp_mode);

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, event_time_ms, data, was_successful, comment);
    }

    py_newbool(py_retval(), was_successful);

    return IS_TRUE;
}

bool py_pdo_del(int argc, py_Ref argv)
{
    int         can_id;
    bool_t      show_output;
    const char* comment;
    disp_mode_t disp_mode = SILENT;

    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_bool);
    PY_CHECK_ARG_TYPE(2, tp_str);

    can_id      = py_toint(py_arg(0));
    show_output = py_tobool(py_arg(1));
    comment     = py_tostr(py_arg(2));

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, 0, 0, IS_TRUE, comment);
    }

    py_newbool(py_retval(), pdo_del(can_id, disp_mode));
    return IS_TRUE;
}
