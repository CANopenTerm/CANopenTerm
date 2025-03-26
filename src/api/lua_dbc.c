/** @file lua_dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua_dbc.h"
#include "core.h"
#include "dbc.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"

int lua_dbc_decode(lua_State* L)
{
    int can_id = luaL_checkinteger(L, 1);
    uint64 data = lua_tointeger(L, 2);
    const char* result = dbc_decode(can_id, data);

    lua_pushstring(L, result);

    return 1;
}

int lua_dbc_find_id_by_name(lua_State* L)
{
    const char* search = luaL_checkstring(L, 1);
    uint32 id;
    status_t status;

    status = dbc_find_id_by_name(&id, search);
    if (ALL_OK == status)
    {
        lua_pushinteger(L, id);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int lua_dbc_load(lua_State* L)
{
    char* filename = (char*)luaL_checkstring(L, 1);
    status_t status;

    dbc_unload();
    status = dbc_load(filename);
    if (ALL_OK == status)
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}

void lua_register_dbc_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_dbc_decode);
    lua_setglobal(core->L, "dbc_decode");
    lua_pushcfunction(core->L, lua_dbc_find_id_by_name);
    lua_setglobal(core->L, "dbc_find_id_by_name");
    lua_pushcfunction(core->L, lua_dbc_load);
    lua_setglobal(core->L, "dbc_load");
}
