/** @file lua_pdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua_pdo.h"
#include "core.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"
#include "pdo.h"

extern void pdo_print_result(uint16 can_id, uint32 event_time_ms, uint64 data, bool_t was_successful, const char* comment);

int lua_pdo_add(lua_State* L)
{
    int         can_id        = luaL_checkinteger(L, 1);
    int         event_time_ms = luaL_checkinteger(L, 2);
    int         length        = luaL_checkinteger(L, 3);
    uint64      data          = lua_tointeger(L, 4);
    bool_t      show_output   = lua_toboolean(L, 5);
    const char* comment       = lua_tostring(L, 6);
    bool_t      was_successful;
    disp_mode_t disp_mode = SILENT;

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    was_successful = pdo_add(can_id, event_time_ms, length, data, disp_mode);

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, event_time_ms, data, was_successful, comment);
    }

    lua_pushboolean(L, was_successful);

    return 1;
}

int lua_pdo_del(lua_State* L)
{
    int         can_id      = luaL_checkinteger(L, 1);
    bool_t      show_output = lua_toboolean(L, 2);
    const char* comment     = lua_tostring(L, 3);
    disp_mode_t disp_mode   = SILENT;

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, 0, 0, IS_TRUE, comment);
    }

    lua_pushboolean(L, pdo_del(can_id, disp_mode));
    return 1;
}

void lua_register_pdo_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_pdo_add);
    lua_setglobal(core->L, "pdo_add");

    lua_pushcfunction(core->L, lua_pdo_del);
    lua_setglobal(core->L, "pdo_del");
}
