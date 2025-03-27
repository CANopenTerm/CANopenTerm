/** @file lua_widget.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"
#include "tachometer.h"
#include "window.h"

int lua_clear_window(lua_State* L)
{
    window_clear();
    return 0;
}

int lua_hide_window(lua_State* L)
{
    window_hide();
    return 0;
}

int lua_show_window(lua_State* L)
{
    window_show();
    return 0;
}

int lua_update_window(lua_State* L)
{
    window_update();
    return 0;
}

int lua_widget_tachometer(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 size = luaL_checkinteger(L, 3);
    const uint32 max = luaL_checkinteger(L, 4);
    uint32 rpm = luaL_checkinteger(L, 5);

    widget_tachometer(pos_x, pos_y, size, max, rpm);
    return 0;
}

void lua_register_widget_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_clear_window);
    lua_setglobal(core->L, "clear_window");
    lua_pushcfunction(core->L, lua_hide_window);
    lua_setglobal(core->L, "hide_window");
    lua_pushcfunction(core->L, lua_show_window);
    lua_setglobal(core->L, "show_window");
    lua_pushcfunction(core->L, lua_update_window);
    lua_setglobal(core->L, "update_window");
    lua_pushcfunction(core->L, lua_widget_tachometer);
    lua_setglobal(core->L, "widget_tachometer");
}
