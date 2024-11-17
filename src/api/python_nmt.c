/** @file python_nmt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "nmt.h"
#include "os.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

extern void nmt_print_error(const char* reason, nmt_command_t command, disp_mode_t disp_mode);

bool py_nmt_send_command(int argc, py_Ref argv);

void python_nmt_init(core_t *core)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "nmt_send_command(node_id, command, show_output=False, comment=\"\")", py_nmt_send_command);
}

bool py_nmt_send_command(int argc, py_Ref argv)
{
    uint32      status;
    disp_mode_t disp_mode = SILENT;
    int         node_id;
    int         command;
    bool_t      show_output;
    const char* comment;

    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_bool);
    PY_CHECK_ARG_TYPE(3, tp_str);

    node_id     = py_toint(py_arg(0));
    command     = py_toint(py_arg(1));
    show_output = py_tobool(py_arg(2));
    comment     = py_tostr(py_arg(3));

    limit_node_id((uint8*)&node_id);

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    switch (command)
    {
        case NMT_OPERATIONAL:
        case NMT_STOP:
        case NMT_PRE_OPERATIONAL:
        case NMT_RESET_NODE:
        case NMT_RESET_COMM:
            status = nmt_send_command(node_id, command, disp_mode, comment);
            if (0 == status)
            {
                py_newbool(py_retval(), IS_TRUE);
            }
            else
            {
                py_newbool(py_retval(), IS_FALSE);
            }
            break;
        default:
            nmt_print_error("Unknown NMT command", command, disp_mode);
            py_newbool(py_retval(), IS_FALSE);
            break;
    }

    return IS_TRUE;
}
