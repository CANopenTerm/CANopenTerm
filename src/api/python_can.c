/** @file python_can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "dict.h"
#include "os.h"
#include "pocketpy.h"

typedef bool (*py_CFunction)(int argc, py_Ref argv);

bool py_dict_lookup_raw(int argc, py_Ref argv);
bool py_can_write(int argc, py_Ref argv);
bool py_can_read(int argc, py_Ref argv);
bool py_can_flush(int argc, py_Ref argv);

void python_can_init(void)
{
    py_GlobalRef mod = py_getmodule("__main__");

    py_bind(mod, "dict_lookup_raw(can_id, data_length, data=0)", py_dict_lookup_raw);
    py_bind(mod, "can_write(can_id, data_length, data=0, is_extended=False, show_output=False, comment=\"\")", py_can_write);

    py_bindfunc(mod, "can_read", py_can_read);
    py_bindfunc(mod, "can_flush", py_can_flush);
}

bool py_dict_lookup_raw(int argc, py_Ref argv)
{
    uint32        can_status;
    int           can_id;
    int           length;
    uint64        data;
    can_message_t message     = {0};
    const char*   description = NULL;

    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);

    can_id          = py_toint(py_arg(0));
    length          = py_toint(py_arg(1));
    data            = py_toint(py_arg(2));
    message.id      = can_id;
    message.length  = length;
    message.data[0] = (data >> 56) & 0xFF;
    message.data[1] = (data >> 48) & 0xFF;
    message.data[2] = (data >> 40) & 0xFF;
    message.data[3] = (data >> 32) & 0xFF;
    message.data[4] = (data >> 24) & 0xFF;
    message.data[5] = (data >> 16) & 0xFF;
    message.data[6] = (data >> 8) & 0xFF;
    message.data[7] = data & 0xFF;
    description     = dict_lookup_raw(&message);

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

bool py_can_write(int argc, py_Ref argv)
{
    uint32        can_status;
    int           can_id;
    int           length;
    uint64        data;
    bool_t        is_extended;
    bool_t        show_output;
    const char*   comment;
    can_message_t message   = {0};
    disp_mode_t   disp_mode = SILENT;

    PY_CHECK_ARGC(6);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_bool);
    PY_CHECK_ARG_TYPE(4, tp_bool);
    PY_CHECK_ARG_TYPE(5, tp_str);

    can_id          = py_toint(py_arg(0));
    length          = py_toint(py_arg(1));
    data            = py_toint(py_arg(2));
    is_extended     = py_tobool(py_arg(3));
    show_output     = py_tobool(py_arg(4));
    comment         = py_tostr(py_arg(5));
    message.id      = can_id;
    message.length  = length;
    message.data[0] = (data >> 56) & 0xFF;
    message.data[1] = (data >> 48) & 0xFF;
    message.data[2] = (data >> 40) & 0xFF;
    message.data[3] = (data >> 32) & 0xFF;
    message.data[4] = (data >> 24) & 0xFF;
    message.data[5] = (data >> 16) & 0xFF;
    message.data[6] = (data >> 8) & 0xFF;
    message.data[7] = data & 0xFF;

    if (IS_TRUE == is_extended)
    {
        message.is_extended = IS_TRUE;
    }
    else
    {
        message.is_extended = IS_FALSE;
    }

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    can_status = can_write(&message, disp_mode, comment);

    if (0 == can_status)
    {
        if (SCRIPT_MODE == disp_mode)
        {
            int  i;
            char buffer[34] = {0};

            if (NULL == comment)
            {
                comment = "-";
            }

            os_strlcpy(buffer, comment, 33);
            for (i = os_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            os_print(LIGHT_BLACK, "CAN ");
            os_print(DEFAULT_COLOR, "     0x%02X   -       -         %03u     ", can_id, length);
            os_print(LIGHT_GREEN, "SUCC    ");
            os_print(DARK_MAGENTA, "%s ", buffer);
            os_print(DEFAULT_COLOR, "Write: 0x%" PRIx64 "\n", data);
        }
        py_newbool(py_retval(), IS_TRUE);
    }
    else
    {
        can_print_error(can_id, can_get_error_message(can_status), disp_mode);
        py_newbool(py_retval(), IS_FALSE);
    }

    return IS_TRUE;
}

bool py_can_read(int argc, py_Ref argv)
{
    can_message_t message = {0};
    status_t      status;
    uint32        length;
    uint64        data = 0;
    ;

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

bool py_can_flush(int argc, py_Ref argv)
{
    PY_CHECK_ARGC(0);
    can_flush();
    return IS_TRUE;
}