/** @file python_widget.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "python_widget.h"
#include "core.h"
#include "os.h"
#include "pocketpy.h"
#include "tachometer.h"
#include "window.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_window_clear(int argc, py_Ref argv);
bool py_window_hide(int argc, py_Ref argv);
bool py_window_show(int argc, py_Ref argv);
bool py_window_update(int argc, py_Ref argv);
bool py_widget_tachometer(int argc, py_Ref argv);

void python_widget_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bindfunc(mod, "window_clear", py_window_clear);
    py_bindfunc(mod, "window_hide", py_window_hide);
    py_bindfunc(mod, "window_show", py_window_show);
    py_bindfunc(mod, "window_update", py_window_update);
    py_bindfunc(mod, "widget_tachometer", py_widget_tachometer);
}

bool py_window_clear(int argc, py_Ref argv)
{
    window_clear();
    py_newnone(py_retval());
    return true;
}

bool py_window_hide(int argc, py_Ref argv)
{
    window_hide();
    py_newnone(py_retval());
    return true;
}

bool py_window_show(int argc, py_Ref argv)
{
    window_show();
    py_newnone(py_retval());
    return true;
}

bool py_window_update(int argc, py_Ref argv)
{
    if (CORE_QUIT == window_update())
    {
        window_clear();
        window_update();
        window_hide();
        py_newbool(py_retval(), false);
        return true;
    }
    py_newbool(py_retval(), true);
    return true;
}

bool py_widget_tachometer(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 size;
    uint32 max;
    uint32 rpm;

    PY_CHECK_ARGC(5);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    size = (uint32)py_toint(py_arg(2));
    max = (uint32)py_toint(py_arg(3));
    rpm = (uint32)py_toint(py_arg(4));

    widget_tachometer(pos_x, pos_y, size, (const uint32)max, rpm);

    py_newnone(py_retval());
    return true;
}
