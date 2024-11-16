/** @file python_can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "os.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_can_read(int argc, py_Ref argv);

void python_can_init(core_t *core)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bindfunc(mod, "can_read", py_can_read);
}

bool py_can_read(int argc, py_Ref argv)
{
    can_message_t message = { 0 };
    status_t      status;
    uint32        length;
    uint64        data = 0;;

    PY_CHECK_ARGC(0);

    status = can_read(&message);
    if (ALL_OK == status)
    {
        length = message.length;

        if (length > 8)
        {
            length = 8;
        }

        os_memcpy(&data, &message.data, sizeof(uint64));

        py_newtuple(py_retval(), 4);

        py_newint(py_r0(), message.id);
        py_newint(py_r1(), length);
        py_newint(py_r2(), data);
        py_newint(py_r3(), message.timestamp_us);

        py_tuple_setitem(py_retval(), 0, py_r0());
        py_tuple_setitem(py_retval(), 1, py_r1());
        py_tuple_setitem(py_retval(), 2, py_r2());
        py_tuple_setitem(py_retval(), 3, py_r3());
    }
    else
    {
        py_newnone(py_retval());
    }

    return IS_TRUE;
}
