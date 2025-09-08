/** @file lua_widget.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ascii.h"
#include "bargraph.h"
#include "core.h"
#include "lauxlib.h"
#include "led.h"
#include "lua.h"
#include "os.h"
#include "tachometer.h"
#include "window.h"

int lua_window_clear(lua_State* L)
{
    window_clear();
    return 0;
}

int lua_window_is_shown(lua_State* L)
{
    int is_shown = (int)window_is_shown();

    lua_pushboolean(L, is_shown);
    return 1;
}

int lua_window_hide(lua_State* L)
{
    window_hide();
    return 0;
}

int lua_window_get_resolution(lua_State* L)
{
    uint32 width, height;
    window_get_resolution(&width, &height);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);

    return 2;
}

int lua_window_show(lua_State* L)
{
    window_show();
    return 0;
}

int lua_window_update(lua_State* L)
{
    extern core_t* core;
    bool render = true;

    if (lua_isboolean(L, 1))
    {
        render = (bool)lua_toboolean(L, 1);
    }

    if (CORE_QUIT == window_update(render))
    {
        core->is_abort = true;
        window_clear();
        window_update(render);
        window_hide();
        os_console_show();
        lua_pushstring(L, "Widget window closed: script stopped.");
        return 0;
    }
    return 0;
}

int lua_widget_bargraph(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 width = luaL_checkinteger(L, 3);
    uint32 height = luaL_checkinteger(L, 4);
    const uint32 max = luaL_checkinteger(L, 5);
    uint32 value = luaL_checkinteger(L, 6);

    widget_bargraph(pos_x, pos_y, width, height, max, value);
    return 0;
}

int lua_widget_led(lua_State* L)
{
    bool state = false;

    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 size = luaL_checkinteger(L, 3);

    if (lua_isboolean(L, 4))
    {
        state = (bool)lua_toboolean(L, 4);
    }

    widget_led(pos_x, pos_y, size, state);
    return 0;
}

int lua_widget_print(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    uint32 scale = luaL_optinteger(L, 4, 1);

    widget_print(pos_x, pos_y, DRAW_WHITE, scale, "%s", str);
    return 0;
}

int lua_widget_tachometer(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 size = luaL_checkinteger(L, 3);
    const uint32 max = luaL_checkinteger(L, 4);
    uint32 value = luaL_checkinteger(L, 5);

    widget_tachometer(pos_x, pos_y, size, max, value);
    return 0;
}

void lua_register_widget_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_window_clear);
    lua_setglobal(core->L, "window_clear");
    lua_pushcfunction(core->L, lua_window_is_shown);
    lua_setglobal(core->L, "window_is_shown");
    lua_pushcfunction(core->L, lua_window_hide);
    lua_setglobal(core->L, "window_hide");
    lua_pushcfunction(core->L, lua_window_get_resolution);
    lua_setglobal(core->L, "window_get_resolution");
    lua_pushcfunction(core->L, lua_window_show);
    lua_setglobal(core->L, "window_show");
    lua_pushcfunction(core->L, lua_window_update);
    lua_setglobal(core->L, "window_update");
    lua_pushcfunction(core->L, lua_widget_bargraph);
    lua_setglobal(core->L, "widget_bargraph");
    lua_pushcfunction(core->L, lua_widget_led);
    lua_setglobal(core->L, "widget_led");
    lua_pushcfunction(core->L, lua_widget_print);
    lua_setglobal(core->L, "widget_print");
    lua_pushcfunction(core->L, lua_widget_tachometer);
    lua_setglobal(core->L, "widget_tachometer");
}
