/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua.h"
#include "lauxlib.h"
#include "can.h"
#include "core.h"
#include "os.h"

void limit_node_id(uint8* node_id)
{
    if (*node_id > 0x7f)
    {
        *node_id = 0x7f;
    }
}

int lua_can_write(lua_State* L)
{
    uint32        can_status;
    int           can_id      = luaL_checkinteger(L, 1);
    int           length      = luaL_checkinteger(L, 2);
    uint64        data        = lua_tointeger(L, 3);
    bool_t        is_extended = lua_toboolean(L, 4);
    bool_t        show_output = lua_toboolean(L, 5);
    const char*   comment     = lua_tostring(L, 6);
    can_message_t message     = { 0 };
    disp_mode_t   disp_mode   = SILENT;

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
            char buffer[34] = { 0 };

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
            os_print(DEFAULT_COLOR, "Write: 0x%16X%08X\n", data);
        }
        lua_pushboolean(L, 1);
    }
    else
    {
        can_print_error(can_id, can_get_error_message(can_status), disp_mode);
        lua_pushboolean(L, 0);
    }

    return 1;
}

int lua_can_read(lua_State* L)
{
    can_message_t message = { 0 };
    status_t      status;
    uint32        length;
    uint64        data    = 0;;

    status = can_read(&message);
    if (ALL_OK == status)
    {
        length = message.length;

        if (length > 8)
        {
            length = 8;
        }

        os_memcpy(&data, &message.data, sizeof(uint64));

        lua_pushinteger(L, message.id);
        lua_pushinteger(L, length);
        lua_pushinteger(L, data);
        lua_pushinteger(L, message.timestamp_us);
        return 4;
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }
}

void lua_register_can_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_can_write);
    lua_setglobal(core->L, "can_write");
    lua_pushcfunction(core->L, lua_can_read);
    lua_setglobal(core->L, "can_read");
}

bool_t is_can_initialised(core_t* core)
{
    if (NULL == core)
    {
        return IS_FALSE;
    }

    return core->is_can_initialised;
}

void can_print_error(uint32 can_id, const char* reason, disp_mode_t disp_mode)
{
    if (SCRIPT_MODE != disp_mode)
    {
        return;
    }

    os_print(LIGHT_BLACK, "CAN ");
    os_print(DEFAULT_COLOR, "     0x%02X   -       -         -       ", can_id);
    os_print(LIGHT_RED, "FAIL    ");

    if (NULL != reason)
    {
        os_print(DEFAULT_COLOR, "%s\n", reason);
    }
    else
    {
        os_print(DEFAULT_COLOR, "-\n");
    }
}
