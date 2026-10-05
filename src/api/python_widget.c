/** @file python_widget.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "python_widget.h"
#include "ascii.h"
#include "bargraph.h"
#include "core.h"
#include "input.h"
#include "led.h"
#include "os.h"
#include "oscilloscope.h"
#include "palette.h"
#include "tachometer.h"
#include "toggle.h"
#include "window.h"
#include <pocketpy.h>

typedef bool (*py_CFunction)(int argc, py_Ref argv);

#define MAX_OSCILLOSCOPE_BUFFERS 16

static oscilloscope_ringbuffer_t* g_py_oscilloscope_buffers[MAX_OSCILLOSCOPE_BUFFERS] = {NULL};

bool py_window_clear(int argc, py_Ref argv);
bool py_window_is_shown(int argc, py_Ref argv);
bool py_window_hide(int argc, py_Ref argv);
bool py_window_get_resolution(int argc, py_Ref argv);
bool py_window_resize(int argc, py_Ref argv);
bool py_window_show(int argc, py_Ref argv);
bool py_window_update(int argc, py_Ref argv);
bool py_widget_bargraph(int argc, py_Ref argv);
bool py_widget_led(int argc, py_Ref argv);
bool py_widget_print(int argc, py_Ref argv);
bool py_widget_tachometer(int argc, py_Ref argv);
bool py_widget_theme(int argc, py_Ref argv);
bool py_widget_input_register(int argc, py_Ref argv);
bool py_widget_input(int argc, py_Ref argv);
bool py_widget_input_set_callback(int argc, py_Ref argv);
bool py_widget_input_get_text(int argc, py_Ref argv);
bool py_widget_input_set_text(int argc, py_Ref argv);
bool py_widget_input_set_position(int argc, py_Ref argv);
bool py_widget_input_unregister(int argc, py_Ref argv);
bool py_widget_toggle_register(int argc, py_Ref argv);
bool py_widget_toggle(int argc, py_Ref argv);
bool py_widget_toggle_set_callback(int argc, py_Ref argv);
bool py_widget_toggle_get_state(int argc, py_Ref argv);
bool py_widget_toggle_set_state(int argc, py_Ref argv);
bool py_widget_toggle_unregister(int argc, py_Ref argv);
bool py_oscilloscope_buffer_create(int argc, py_Ref argv);
bool py_oscilloscope_buffer_destroy(int argc, py_Ref argv);
bool py_oscilloscope_buffer_push(int argc, py_Ref argv);
bool py_widget_oscilloscope(int argc, py_Ref argv);
bool py_widget_oscilloscope_2ch(int argc, py_Ref argv);

void python_widget_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "window_update(render=True)", py_window_update);
    py_bind(mod, "widget_print(pos_x, pos_y, str, scale=1)", py_widget_print);

    py_bindfunc(mod, "window_clear", py_window_clear);
    py_bindfunc(mod, "window_is_shown", py_window_is_shown);
    py_bindfunc(mod, "window_hide", py_window_hide);
    py_bindfunc(mod, "window_get_resolution", py_window_get_resolution);
    py_bindfunc(mod, "window_resize", py_window_resize);
    py_bindfunc(mod, "window_show", py_window_show);
    py_bindfunc(mod, "widget_bargraph", py_widget_bargraph);
    py_bindfunc(mod, "widget_led", py_widget_led);
    py_bindfunc(mod, "widget_tachometer", py_widget_tachometer);
    py_bindfunc(mod, "widget_theme", py_widget_theme);
    py_bindfunc(mod, "widget_input_register", py_widget_input_register);
    py_bindfunc(mod, "widget_input", py_widget_input);
    py_bindfunc(mod, "widget_input_set_callback", py_widget_input_set_callback);
    py_bindfunc(mod, "widget_input_get_text", py_widget_input_get_text);
    py_bindfunc(mod, "widget_input_set_text", py_widget_input_set_text);
    py_bindfunc(mod, "widget_input_set_position", py_widget_input_set_position);
    py_bindfunc(mod, "widget_input_unregister", py_widget_input_unregister);
    py_bindfunc(mod, "widget_toggle_register", py_widget_toggle_register);
    py_bindfunc(mod, "widget_toggle", py_widget_toggle);
    py_bindfunc(mod, "widget_toggle_set_callback", py_widget_toggle_set_callback);
    py_bindfunc(mod, "widget_toggle_get_state", py_widget_toggle_get_state);
    py_bindfunc(mod, "widget_toggle_set_state", py_widget_toggle_set_state);
    py_bindfunc(mod, "widget_toggle_unregister", py_widget_toggle_unregister);
    py_bindfunc(mod, "oscilloscope_buffer_create", py_oscilloscope_buffer_create);
    py_bindfunc(mod, "oscilloscope_buffer_destroy", py_oscilloscope_buffer_destroy);
    py_bindfunc(mod, "oscilloscope_buffer_push", py_oscilloscope_buffer_push);
    py_bindfunc(mod, "widget_oscilloscope", py_widget_oscilloscope);
    py_bindfunc(mod, "widget_oscilloscope_2ch", py_widget_oscilloscope_2ch);
}

bool py_window_clear(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);

    window_clear();
    py_newnone(py_retval());
    return true;
}

bool py_window_is_shown(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);

    py_newbool(py_retval(), window_is_shown());
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

bool py_window_resize(int argc, py_Ref argv)
{
    uint32 width, height;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    width = (uint32)py_toint(py_arg(0));
    height = (uint32)py_toint(py_arg(1));

    window_resize(width, height);

    py_newnone(py_retval());
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
        os_console_show();
        py_newbool(py_retval(), false);
        return true;
    }
    py_newbool(py_retval(), true);
    return true;
}

bool py_widget_bargraph(int argc, py_Ref argv)
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

    widget_bargraph(pos_x, pos_y, width, height, max, value);

    py_newnone(py_retval());
    return true;
}

bool py_widget_led(int argc, py_Ref argv)
{
    bool state = false;

    uint32 pos_x;
    uint32 pos_y;
    uint32 size;

    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_bool);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    size = (uint32)py_toint(py_arg(2));
    state = (bool)py_tobool(py_arg(3));

    widget_led(pos_x, pos_y, size, state);

    py_newnone(py_retval());
    return true;
}

bool py_widget_print(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 scale;
    const char* str;

    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_str);
    PY_CHECK_ARG_TYPE(3, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    str = py_tostr(py_arg(2));
    scale = (uint32)py_toint(py_arg(3));

    widget_print(pos_x, pos_y, DRAW_COLOR, scale, "%s", str);

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

bool py_widget_theme(int argc, py_Ref argv)
{
    pal_theme_t theme;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    theme = (pal_theme_t)py_toint(py_arg(0));

    palette_set_theme(theme);

    py_newnone(py_retval());
    return true;
}

bool py_oscilloscope_buffer_create(int argc, py_Ref argv)
{
    uint32 min_value;
    uint32 max_value;
    uint32 size = 1024;
    int buffer_id = -1;
    int i;

    if (argc < 2 || argc > 3)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    min_value = (uint32)py_toint(py_arg(0));
    max_value = (uint32)py_toint(py_arg(1));

    if (argc >= 3)
    {
        PY_CHECK_ARG_TYPE(2, tp_int);
        size = (uint32)py_toint(py_arg(2));
    }

    for (i = 0; i < MAX_OSCILLOSCOPE_BUFFERS; i++)
    {
        if (g_py_oscilloscope_buffers[i] == NULL)
        {
            g_py_oscilloscope_buffers[i] = oscilloscope_buffer_create(min_value, max_value, size);
            if (g_py_oscilloscope_buffers[i])
            {
                buffer_id = i;
            }
            break;
        }
    }

    py_newint(py_retval(), buffer_id);
    return true;
}

bool py_oscilloscope_buffer_destroy(int argc, py_Ref argv)
{
    int buffer_id;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    buffer_id = (int)py_toint(py_arg(0));

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (g_py_oscilloscope_buffers[buffer_id])
        {
            oscilloscope_buffer_destroy(g_py_oscilloscope_buffers[buffer_id]);
            g_py_oscilloscope_buffers[buffer_id] = NULL;
        }
    }

    py_newnone(py_retval());
    return true;
}

bool py_oscilloscope_buffer_push(int argc, py_Ref argv)
{
    int buffer_id;
    uint32 value;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    buffer_id = (int)py_toint(py_arg(0));
    value = (uint32)py_toint(py_arg(1));

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (g_py_oscilloscope_buffers[buffer_id])
        {
            oscilloscope_buffer_push(g_py_oscilloscope_buffers[buffer_id], value);
        }
    }

    py_newnone(py_retval());
    return true;
}

bool py_widget_oscilloscope(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    int buffer_id;
    uint32 current_value;
    const char* label = "OSC";
    uint64 time_window_ms = 0;

    if (argc < 7 || argc > 8)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_int);
    PY_CHECK_ARG_TYPE(5, tp_int);
    PY_CHECK_ARG_TYPE(6, tp_str);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    width = (uint32)py_toint(py_arg(2));
    height = (uint32)py_toint(py_arg(3));
    buffer_id = (int)py_toint(py_arg(4));
    current_value = (uint32)py_toint(py_arg(5));
    label = py_tostr(py_arg(6));

    if (argc > 7)
    {
        PY_CHECK_ARG_TYPE(7, tp_int);
        time_window_ms = (uint64)py_toint(py_arg(7));
    }

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (g_py_oscilloscope_buffers[buffer_id])
        {
            widget_oscilloscope(pos_x, pos_y, width, height, g_py_oscilloscope_buffers[buffer_id], current_value, label, time_window_ms);
        }
    }

    py_newnone(py_retval());
    return true;
}

bool py_widget_oscilloscope_2ch(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    int buffer_id1;
    int buffer_id2;
    uint32 current_value1;
    uint32 current_value2;
    const char* label1 = "OSC1";
    const char* label2 = "OSC2";
    uint64 time_window_ms = 0;

    if (argc < 10 || argc > 11)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_int);
    PY_CHECK_ARG_TYPE(5, tp_int);
    PY_CHECK_ARG_TYPE(6, tp_str);
    PY_CHECK_ARG_TYPE(7, tp_int);
    PY_CHECK_ARG_TYPE(8, tp_int);
    PY_CHECK_ARG_TYPE(9, tp_str);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    width = (uint32)py_toint(py_arg(2));
    height = (uint32)py_toint(py_arg(3));
    buffer_id1 = (int)py_toint(py_arg(4));
    current_value1 = (uint32)py_toint(py_arg(5));
    label1 = py_tostr(py_arg(6));
    buffer_id2 = (int)py_toint(py_arg(7));
    current_value2 = (uint32)py_toint(py_arg(8));
    label2 = py_tostr(py_arg(9));

    if (argc > 10)
    {
        PY_CHECK_ARG_TYPE(10, tp_int);
        time_window_ms = (uint64)py_toint(py_arg(10));
    }

    if (buffer_id1 >= 0 && buffer_id1 < MAX_OSCILLOSCOPE_BUFFERS &&
        buffer_id2 >= 0 && buffer_id2 < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (g_py_oscilloscope_buffers[buffer_id1] && g_py_oscilloscope_buffers[buffer_id2])
        {
            widget_oscilloscope_2ch(pos_x, pos_y, width, height,
                                     g_py_oscilloscope_buffers[buffer_id1], current_value1, label1,
                                     g_py_oscilloscope_buffers[buffer_id2], current_value2, label2,
                                     time_window_ms);
        }
    }

    py_newnone(py_retval());
    return true;
}

bool py_widget_input_register(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    const char* initial_text = "";

    if (argc < 4 || argc > 5)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    width = (uint32)py_toint(py_arg(2));
    height = (uint32)py_toint(py_arg(3));

    if (argc > 4)
    {
        PY_CHECK_ARG_TYPE(4, tp_str);
        initial_text = py_tostr(py_arg(4));
    }

    uint32 input_id = widget_input_register(pos_x, pos_y, width, height, initial_text);
    py_newint(py_retval(), input_id);
    return true;
}

bool py_widget_input(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    const char* text;
    bool is_active = false;

    if (argc < 5 || argc > 6)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_str);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    width = (uint32)py_toint(py_arg(2));
    height = (uint32)py_toint(py_arg(3));
    text = py_tostr(py_arg(4));

    if (argc > 5)
    {
        PY_CHECK_ARG_TYPE(5, tp_bool);
        is_active = py_tobool(py_arg(5));
    }

    widget_input(pos_x, pos_y, width, height, text, is_active);
    py_newnone(py_retval());
    return true;
}

bool py_widget_input_set_callback(int argc, py_Ref argv)
{
    uint32 input_id;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);

    input_id = (uint32)py_toint(py_arg(0));

    /* Python callbacks are more complex - for now, we'll support basic input without callbacks */
    /* Full callback support would require storing Python function references */
    py_newbool(py_retval(), false);
    return true;
}

bool py_widget_input_get_text(int argc, py_Ref argv)
{
    uint32 input_id;
    const char* text;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    input_id = (uint32)py_toint(py_arg(0));
    text = widget_input_get_text(input_id);
    py_newstr(py_retval(), text);
    return true;
}

bool py_widget_input_set_text(int argc, py_Ref argv)
{
    uint32 input_id;
    const char* text;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_str);

    input_id = (uint32)py_toint(py_arg(0));
    text = py_tostr(py_arg(1));

    widget_input_set_text(input_id, text);
    py_newnone(py_retval());
    return true;
}

bool py_widget_input_set_position(int argc, py_Ref argv)
{
    uint32 input_id;
    uint32 pos_x;
    uint32 pos_y;

    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);

    input_id = (uint32)py_toint(py_arg(0));
    pos_x = (uint32)py_toint(py_arg(1));
    pos_y = (uint32)py_toint(py_arg(2));

    widget_input_set_position(input_id, pos_x, pos_y);
    py_newnone(py_retval());
    return true;
}

bool py_widget_input_unregister(int argc, py_Ref argv)
{
    uint32 input_id;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    input_id = (uint32)py_toint(py_arg(0));
    widget_input_unregister(input_id);
    py_newnone(py_retval());
    return true;
}

bool py_widget_toggle_register(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 size;
    bool initial_state = false;

    if (argc < 3 || argc > 4)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    size = (uint32)py_toint(py_arg(2));

    if (argc > 3)
    {
        PY_CHECK_ARG_TYPE(3, tp_bool);
        initial_state = py_tobool(py_arg(3));
    }

    uint32 toggle_id = widget_toggle_register(pos_x, pos_y, size, initial_state);
    py_newint(py_retval(), toggle_id);
    return true;
}

bool py_widget_toggle(int argc, py_Ref argv)
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 size;
    bool state = false;

    if (argc < 3 || argc > 4)
    {
        return false;
    }

    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);

    pos_x = (uint32)py_toint(py_arg(0));
    pos_y = (uint32)py_toint(py_arg(1));
    size = (uint32)py_toint(py_arg(2));

    if (argc > 3)
    {
        PY_CHECK_ARG_TYPE(3, tp_bool);
        state = py_tobool(py_arg(3));
    }

    widget_toggle(pos_x, pos_y, size, state);
    py_newnone(py_retval());
    return true;
}

bool py_widget_toggle_set_callback(int argc, py_Ref argv)
{
    uint32 toggle_id;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);

    toggle_id = (uint32)py_toint(py_arg(0));

    /* Python callbacks are more complex - for now, we'll support basic toggle without callbacks */
    /* Full callback support would require storing Python function references */
    py_newbool(py_retval(), false);
    return true;
}

bool py_widget_toggle_get_state(int argc, py_Ref argv)
{
    uint32 toggle_id;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    toggle_id = (uint32)py_toint(py_arg(0));
    py_newbool(py_retval(), widget_toggle_get_state(toggle_id));
    return true;
}

bool py_widget_toggle_set_state(int argc, py_Ref argv)
{
    uint32 toggle_id;
    bool state;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_bool);

    toggle_id = (uint32)py_toint(py_arg(0));
    state = py_tobool(py_arg(1));

    widget_toggle_set_state(toggle_id, state);
    py_newnone(py_retval());
    return true;
}

bool py_widget_toggle_unregister(int argc, py_Ref argv)
{
    uint32 toggle_id;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    toggle_id = (uint32)py_toint(py_arg(0));
    widget_toggle_unregister(toggle_id);

    py_newnone(py_retval());
    return true;
}

