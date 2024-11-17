/** @file lua_sdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "dict.h"
#include "lua.h"
#include "lauxlib.h"
#include "lua_sdo.h"
#include "os.h"
#include "sdo.h"

extern bool_t is_printable_string(const char *str, size_t length);

int lua_sdo_lookup_abort_code(lua_State *L)
{
    int         abort_code = luaL_checkinteger(L, 1);
    const char* description = sdo_lookup_abort_code(abort_code);

    lua_pushstring(L, (const char *)description);
    return 1;
}

int lua_sdo_read(lua_State *L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode = SILENT;
    sdo_state_t   sdo_state;
    int           node_id = luaL_checkinteger(L, 1);
    int           index = luaL_checkinteger(L, 2);
    int           sub_index = luaL_checkinteger(L, 3);
    bool_t        show_output = lua_toboolean(L, 4);
    const char*   comment = lua_tostring(L, 5);
    char          str_buffer[5] = { 0 };
    uint32        result;

    limit_node_id((uint8 *)&node_id);

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
            lua_pushstring(L, (const char *)sdo_response.data);
            lua_pushstring(L, (const char *)sdo_response.data);
            break;
        case IS_READ_EXPEDITED:
            os_memcpy(&result, &sdo_response.data, sizeof(uint32));
            os_memcpy(&str_buffer, &sdo_response.data, sizeof(uint32));
            lua_pushinteger(L, result);

            if (is_printable_string(str_buffer, sizeof(uint32)))
            {
                lua_pushstring(L, (const char *)str_buffer);
            }
            else
            {
                lua_pushnil(L);
            }
            break;
        default:
        case ABORT_TRANSFER:
            lua_pushnil(L);
            lua_pushnil(L);
            break;
    }

    return 2;
}

int lua_sdo_write(lua_State *L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = SILENT;
    sdo_state_t   sdo_state;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    int           length       = luaL_checkinteger(L, 4);
    int           data         = lua_tointeger(L, 5);
    bool_t        show_output  = lua_toboolean(L, 6);
    const char*   comment      = lua_tostring(L, 7);

    limit_node_id((uint8 *)&node_id);

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
        (void *)&data,
        comment);

    switch (sdo_state)
    {
        case ABORT_TRANSFER:
            lua_pushboolean(L, 0);
            break;
        default:
            lua_pushboolean(L, 1);
            break;
    }

    return 1;
}

int lua_sdo_write_file(lua_State *L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = SILENT;
    int           status;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    const char*   filename     = luaL_checkstring(L, 4);

    if (NULL == filename)
    {
        lua_pushboolean(L, 0);
        return 1;
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
            lua_pushboolean(L, 0);
            break;
        default:
            lua_pushboolean(L, 1);
            break;
    }

    return 1;
}

int lua_sdo_write_string(lua_State *L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = SILENT;
    int           status;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    const char*   data         = luaL_checkstring(L, 4);
    uint32        length       = 0;
    bool_t        show_output  = lua_toboolean(L, 5);
    const char*   comment      = lua_tostring(L, 6);

    if (NULL != data)
    {
        length = os_strlen(data);
    }
    else
    {
        lua_pushboolean(L, 0);
        return 1;
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
        (void *)data,
        comment);

    switch (status)
    {
        case ABORT_TRANSFER:
            lua_pushboolean(L, 0);
            break;
        default:
            lua_pushboolean(L, 1);
            break;
    }

    return 1;
}

int lua_dict_lookup(lua_State *L)
{
    int         index       = luaL_checkinteger(L, 1);
    int         sub_index   = luaL_checkinteger(L, 2);
    const char* description = dict_lookup(index, sub_index);

    if (NULL == description)
    {
        lua_pushnil(L);
    }
    else
    {
        lua_pushstring(L, description);
    }

    return 1;
}

void lua_register_sdo_commands(core_t *core)
{
    lua_pushcfunction(core->L, lua_sdo_lookup_abort_code);
    lua_setglobal(core->L, "sdo_lookup_abort_code");

    lua_pushcfunction(core->L, lua_sdo_read);
    lua_setglobal(core->L, "sdo_read");

    lua_pushcfunction(core->L, lua_sdo_write);
    lua_setglobal(core->L, "sdo_write");

    lua_pushcfunction(core->L, lua_sdo_write_file);
    lua_setglobal(core->L, "sdo_write_file");

    lua_pushcfunction(core->L, lua_sdo_write_string);
    lua_setglobal(core->L, "sdo_write_string");

    lua_pushcfunction(core->L, lua_dict_lookup);
    lua_setglobal(core->L, "dict_lookup");
}
