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
#include "status_bar.h"
#include "tachometer.h"
#include "window.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_window_clear(int argc, py_Ref argv);
bool py_window_hide(int argc, py_Ref argv);
bool py_window_get_resolution(int argc, py_Ref argv);
bool py_window_show(int argc, py_Ref argv);
bool py_window_update(int argc, py_Ref argv);
bool py_widget_status_bar(int argc, py_Ref argv);
bool py_widget_tachometer(int argc, py_Ref argv);

void python_widget_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "window_update(render=True)", py_window_update);

    py_bindfunc(mod, "window_clear", py_window_clear);
    py_bindfunc(mod, "window_hide", py_window_hide);
    py_bindfunc(mod, "window_get_resolution", py_window_get_resolution);
    py_bindfunc(mod, "window_show", py_window_show);
    py_bindfunc(mod, "widget_tachometer", py_widget_tachometer);
}

bool py_window_clear(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);

    window_clear();
    py_newnone(py_retval());
    return true;
}

bool py_window_hide(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);

    window_hide();
    py_newnone(py_retval());
    return true;
}

bool py_window_get_resolution(int argc, py_Ref argv)
{
    uint32 width, height;

    PY_CHECK_ARGC(0);
    window_get_resolution(&width, &height);

    py_newtuple(py_retval(), 2);

    py_newint(py_r0(), width);
    py_newint(py_r1(), height);

    py_tuple_setitem(py_retval(), 0, py_r0());
    py_tuple_setitem(py_retval(), 1, py_r1());

    return true;
}

bool py_window_show(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);

    window_show();
    py_newnone(py_retval());
    return true;
}

bool py_window_update(int argc, py_Ref argv)
{
    extern core_t* core;
    bool render;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bool);

    render = py_tobool(py_arg(0));

    if (CORE_QUIT == window_update(render))
    {
        core->is_abort = true;
        window_clear();
        window_update(render);
        window_hide();
        py_newbool(py_retval(), false);
        return true;
    }
    py_newbool(py_retval(), true);
    return true;
}

bool py_widget_status_bar(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    uint32 max;
    uint32 value;

    PY_CHECK_ARGC(6);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_int);
    PY_CHECK_ARG_TYPE(5, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    width = (uint32)py_toint(py_arg(2));
    height = (uint32)py_toint(py_arg(3));
    max = (uint32)py_toint(py_arg(4));
    value = (uint32)py_toint(py_arg(5));

    widget_status_bar(pos_x, pos_y, width, height, max, value);

    py_newnone(py_retval());
    return true;
}

bool py_widget_tachometer(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 size;
    uint32 max;
    uint32 value;

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
    value = (uint32)py_toint(py_arg(4));

    widget_tachometer(pos_x, pos_y, size, (const uint32)max, value);

    py_newnone(py_retval());
    return true;
}
