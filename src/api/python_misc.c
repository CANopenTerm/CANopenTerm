/** @file python_misc.c
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

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_print_heading(int argc, py_Ref argv);

void python_misc_init(core_t *core)
{
    py_GlobalRef mod = py_getmodule("__main__");
    py_bindfunc(mod, "print_heading", py_print_heading);
}

bool py_print_heading(int argc, py_Ref argv)
{
    const char *heading;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);

    heading = py_tostr(py_arg(0));
    print_heading(heading);
    py_newnone(py_retval());

    return IS_TRUE;
}
