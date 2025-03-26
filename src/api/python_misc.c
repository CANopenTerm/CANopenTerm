/** @file python_misc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "python_misc.h"
#include "core.h"
#include "os.h"
#include "pocketpy.h"
#include "scripts.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_delay_ms(int argc, py_Ref argv);
bool py_key_is_hit(int argc, py_Ref argv);
bool py_print_heading(int argc, py_Ref argv);

void python_misc_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "delay_ms(delay_in_ms, show_output=False, comment=\"\")", py_delay_ms);

    py_bindfunc(mod, "key_is_hit", py_key_is_hit);
    py_bindfunc(mod, "print_heading", py_print_heading);
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

bool py_key_is_hit(int argc, py_Ref argv)
{
    py_newbool(py_retval(), os_key_is_hit());
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
