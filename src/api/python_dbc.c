/** @file python_dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "dbc.h"
#include "os.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_dbc_decode(int argc, py_Ref argv);
bool py_dbc_find_id_by_name(int argc, py_Ref argv);
bool py_dbc_load(int argc, py_Ref argv);

void python_dbc_init(core_t *core)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "dbc_decode(can_id, data=0)", py_dbc_decode);

    py_bindfunc(mod, "dbc_find_id_by_name", py_dbc_find_id_by_name);
    py_bindfunc(mod, "dbc_load", py_dbc_load);
}

bool py_dbc_decode(int argc, py_Ref argv)
{
    int         can_id;
    uint64      data;
    const char* result;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    can_id = py_toint(py_arg(0));
    data   = py_toint(py_arg(1));
    result = dbc_decode(can_id, data);

    py_newstr(py_retval(), result);

    return IS_TRUE;
}

bool py_dbc_find_id_by_name(int argc, py_Ref argv)
{
    const char* search;
    uint32      id;
    status_t    status;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);

    search = py_tostr(py_arg(0));
    status = dbc_find_id_by_name(&id, search);

    if (ALL_OK == status)
    {
        py_newint(py_retval(), id);
    }
    else
    {
        py_newnone(py_retval());
    }

    return IS_TRUE;
}

bool py_dbc_load(int argc, py_Ref argv)
{
    const char* filename;
    status_t    status;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);

    filename = py_tostr(py_arg(0));

    dbc_unload();
    status = dbc_load(filename);
    if (ALL_OK == status)
    {
        py_newbool(py_retval(), IS_TRUE);
    }
    else
    {
        py_newbool(py_retval(), IS_FALSE);
    }

    return IS_TRUE;
}
