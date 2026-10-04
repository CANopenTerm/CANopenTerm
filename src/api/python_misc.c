/** @file python_misc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "python_misc.h"
#include "core.h"
#include "os.h"
#include <pocketpy.h>
#include "scripts.h"
#include "command.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

static core_t* g_py_core = NULL;

bool py_delay_ms(int argc, py_Ref argv);
bool py_console_hide(int argc, py_Ref argv);
bool py_console_show(int argc, py_Ref argv);
bool py_key_is_hit(int argc, py_Ref argv);
bool py_key_send(int argc, py_Ref argv);
bool py_print_heading(int argc, py_Ref argv);
bool py_print_result(int argc, py_Ref argv);
bool py_run_sequence(int argc, py_Ref argv);

void python_misc_init(core_t* core)
{
    py_GlobalRef mod = py_getmodule("__main__");

    g_py_core = core;

    py_bind(mod, "delay_ms(delay_in_ms, show_output=False, comment=\"\")", py_delay_ms);

    py_bindfunc(mod, "console_hide", py_console_hide);
    py_bindfunc(mod, "console_show", py_console_show);
    py_bindfunc(mod, "key_is_hit", py_key_is_hit);
    py_bindfunc(mod, "key_send", py_key_send);
    py_bindfunc(mod, "print_heading", py_print_heading);
    py_bindfunc(mod, "print_result", py_print_result);
    py_bindfunc(mod, "run_sequence", py_run_sequence);
}

bool py_delay_ms(int argc, py_Ref argv)
{
    uint32 delay_in_ms;
    bool show_output;
    const char* comment;

    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_bool);
    PY_CHECK_ARG_TYPE(2, tp_str);

    delay_in_ms = py_toint(py_arg(0));
    show_output = py_tobool(py_arg(1));
    comment = py_tostr(py_arg(2));

    if (0 == delay_in_ms)
    {
        delay_in_ms = 1u;
    }

    if (true == show_output)
    {
        int i;
        char buffer[34] = {0};

        os_print(LIGHT_BLACK, "Delay ");
        os_print(DEFAULT_COLOR, "   -       -       -         -       -       ");

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        os_print(DARK_MAGENTA, "%s ", buffer);
        os_print(DEFAULT_COLOR, "%ums\n", delay_in_ms);
    }

    os_delay(delay_in_ms);

    py_newnone(py_retval());
    return true;
}

bool py_console_hide(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);
    os_console_hide();
    return true;
}

bool py_console_show(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);
    os_console_show();
    return true;
}

bool py_key_is_hit(int argc, py_Ref argv)
{
    py_newbool(py_retval(), os_key_is_hit());
    return true;
}

bool py_key_send(int argc, py_Ref argv)
{
    uint16 key;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    key = (uint16)py_toint(py_arg(0));
    os_key_send(key);

    py_newnone(py_retval());
    return true;
}

bool py_print_heading(int argc, py_Ref argv)
{
    const char* heading;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);

    heading = py_tostr(py_arg(0));
    print_heading(heading);

    py_newnone(py_retval());
    return true;
}

bool py_print_result(int argc, py_Ref argv)
{
    uint8 id;
    uint16 index;
    uint8 sub_index;
    uint32 length;
    bool success;
    const char* comment;
    uint32 data;

    PY_CHECK_ARGC(7);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_bool);
    PY_CHECK_ARG_TYPE(5, tp_str);
    PY_CHECK_ARG_TYPE(6, tp_int);

    id = (uint8)py_toint(py_arg(0));
    index = (uint16)py_toint(py_arg(1));
    sub_index = (uint8)py_toint(py_arg(2));
    length = (uint32)py_toint(py_arg(3));
    success = py_tobool(py_arg(4));
    comment = py_tostr(py_arg(5));
    data = (uint32)py_toint(py_arg(6));

    print_result(id, index, sub_index, length, success, comment, data);
    py_newnone(py_retval());
    return true;
}

bool py_run_sequence(int argc, py_Ref argv)
{
    int i = 0;
    py_Ref item;
    char buffer[512];

    if (NULL == g_py_core)
    {
        py_newbool(py_retval(), false);
        return true;
    }

    /* Support both single string and list of strings */
    if (py_isinstance(py_arg(0), tp_str))
    {
        /* Single command */
        const char* cmd = py_tostr(py_arg(0));
        os_strlcpy(buffer, cmd, sizeof(buffer));
        parse_command(buffer, g_py_core, SCRIPT_MODE);

        py_newbool(py_retval(), true);
        return true;
    }
    else if (py_isinstance(py_arg(0), tp_list))
    {
        /* List of commands */
        while (true)
        {
            item = py_list_getitem(py_arg(0), i);

            if (item == NULL)
            {
                break;
            }

            if (!py_isinstance(item, tp_str))
            {
                py_newbool(py_retval(), false);
                return true;
            }

            os_strlcpy(buffer, py_tostr(item), sizeof(buffer));
            parse_command(buffer, g_py_core, SCRIPT_MODE);
            i++;
        }

        py_newbool(py_retval(), true);
        return true;
    }

    py_newbool(py_retval(), false);
    return true;
}
