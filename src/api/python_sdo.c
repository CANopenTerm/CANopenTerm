/** @file python_sdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "dict.h"
#include "os.h"
#include "pocketpy.h"
#include "sdo.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

extern bool_t is_printable_string(const char* str, size_t length);

bool py_sdo_lookup_abort_code(int argc, py_Ref argv);
bool py_sdo_read(int argc, py_Ref argv);
bool py_sdo_write(int argc, py_Ref argv);
bool py_sdo_write_file(int argc, py_Ref argv);
bool py_sdo_write_string(int argc, py_Ref argv);
bool py_dict_lookup(int argc, py_Ref argv);

void python_sdo_init(core_t *core)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "sdo_read(node_id, index, sub_index, show_output=False, comment=\"\")",                    py_sdo_read);
    py_bind(mod, "sdo_write(node_id, index, sub_index, length, data=0, show_output=False, comment=\"\")",   py_sdo_write);
    py_bind(mod, "sdo_write_string(node_id, index, sub_index, data=\"\", show_output=False, comment=\"\")", py_sdo_write_string);

    py_bindfunc(mod, "sdo_lookup_abort_code", py_sdo_lookup_abort_code);
    py_bindfunc(mod, "sdo_write_file",        py_sdo_write_file);
    py_bindfunc(mod, "dict_lookup",           py_dict_lookup);
}

bool py_sdo_lookup_abort_code(int argc, py_Ref argv)
{
    int         abort_code;
    const char* description;

    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    abort_code  = py_toint(py_arg(0));
    description = sdo_lookup_abort_code(abort_code);

    py_newstr(py_retval(), description);

    return IS_TRUE;
}

bool py_sdo_read(int argc, py_Ref argv)
{
    can_message_t sdo_response  = { 0 };
    disp_mode_t   disp_mode     = SILENT;
    sdo_state_t   sdo_state;
    int           node_id;
    int           index;
    int           sub_index;
    bool_t        show_output;
    const char*   comment;
    uint32        result;

    PY_CHECK_ARGC(5);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_bool);
    PY_CHECK_ARG_TYPE(4, tp_str);

    node_id     = py_toint(py_arg(0));
    index       = py_toint(py_arg(1));
    sub_index   = py_toint(py_arg(2));
    show_output = py_tobool(py_arg(3));
    comment     = py_tostr(py_arg(4));

    limit_node_id((uint8*)&node_id);

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    sdo_state = sdo_read(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint16)sub_index,
        comment);

    switch (sdo_state)
    {
        case IS_READ_SEGMENTED:
            py_newstr(py_retval(), (const char*)sdo_response.data);
            break;
        case IS_READ_EXPEDITED:
            os_memcpy(&result, &sdo_response.data, sizeof(uint32));
            py_newint(py_retval(), result);
            break;
        default:
        case ABORT_TRANSFER:
            py_newnone(py_retval());
            break;
    }

    return IS_TRUE;
}

bool py_sdo_write(int argc, py_Ref argv)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode = SILENT;
    sdo_state_t   sdo_state;
    int           node_id;
    int           index;
    int           sub_index;
    int           length;
    int           data;
    bool_t        show_output;
    const char*   comment;

    PY_CHECK_ARGC(7);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_int);
    PY_CHECK_ARG_TYPE(4, tp_int);
    PY_CHECK_ARG_TYPE(5, tp_bool);
    PY_CHECK_ARG_TYPE(6, tp_str);

    node_id     = py_toint(py_arg(0));
    index       = py_toint(py_arg(1));
    sub_index   = py_toint(py_arg(2));
    length      = py_toint(py_arg(3));
    data        = py_toint(py_arg(4));
    show_output = py_tobool(py_arg(5));
    comment     = py_tostr(py_arg(6));

    limit_node_id((uint8*)&node_id);

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    sdo_state = sdo_write(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        (uint32)length,
        (void*)&data,
        comment);

    switch (sdo_state)
    {
        case ABORT_TRANSFER:
            py_newbool(py_retval(), IS_FALSE);
            break;
        default:
            py_newbool(py_retval(), IS_TRUE);
            break;
    }

    return IS_TRUE;
}

bool py_sdo_write_file(int argc, py_Ref argv)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = SILENT;
    int           status;
    int           node_id;
    int           index;
    int           sub_index;
    const char*   filename;

    PY_CHECK_ARGC(4);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_str);

    node_id   = py_toint(py_arg(0));
    index     = py_toint(py_arg(1));
    sub_index = py_toint(py_arg(2));
    filename  = py_tostr(py_arg(3));

    if (NULL == filename)
    {
        py_newbool(py_retval(), IS_FALSE);
        return IS_TRUE;
    }

    status = sdo_write_block(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        filename,
        NULL);

    switch (status)
    {
        case ABORT_TRANSFER:
            py_newbool(py_retval(), IS_FALSE);
            break;
        default:
            py_newbool(py_retval(), IS_TRUE);
            break;
    }

    return IS_TRUE;
}

bool py_sdo_write_string(int argc, py_Ref argv)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = SILENT;
    int           status;
    int           node_id;
    int           index;
    int           sub_index;
    const char*   data;
    uint32        length       = 0;
    bool_t        show_output;
    const char*   comment;

    PY_CHECK_ARGC(6);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_str);
    PY_CHECK_ARG_TYPE(4, tp_bool);
    PY_CHECK_ARG_TYPE(5, tp_str);

    node_id     = py_toint(py_arg(0));
    index       = py_toint(py_arg(1));
    sub_index   = py_toint(py_arg(2));
    data        = py_tostr(py_arg(3));
    show_output = py_tobool(py_arg(4));
    comment     = py_tostr(py_arg(5));

    if (NULL != data)
    {
        length = os_strlen(data);
    }
    else
    {
        py_newbool(py_retval(), IS_FALSE);
        return IS_TRUE;
    }

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    status = sdo_write_segmented(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        length,
        (void*)data,
        comment);

    switch (status)
    {
        case ABORT_TRANSFER:
            py_newbool(py_retval(), IS_FALSE);
            break;
        default:
            py_newbool(py_retval(), IS_TRUE);
            break;
    }

    return IS_TRUE;
}

bool py_dict_lookup(int argc, py_Ref argv)
{
    int         index;
    int         sub_index;
    const char* description;

    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    index       = py_toint(py_arg(0));
    sub_index   = py_toint(py_arg(1));
    description = dict_lookup(index, sub_index);

    if (NULL == description)
    {
        py_newnone(py_retval());
    }
    else
    {
        py_newstr(py_retval(), description);
    }

    return IS_TRUE;
}
