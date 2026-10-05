/** @file lua_widget.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ascii.h"
#include "bargraph.h"
#include "core.h"
#include "input.h"
#include "lauxlib.h"
#include "led.h"
#include "lua.h"
#include "os.h"
#include "oscilloscope.h"
#include "palette.h"
#include "tachometer.h"
#include "toggle.h"
#include "window.h"

#define MAX_OSCILLOSCOPE_BUFFERS 16

static oscilloscope_ringbuffer_t* oscilloscope_buffers[MAX_OSCILLOSCOPE_BUFFERS] = {NULL};

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

int lua_window_resize(lua_State* L)
{
    uint32 width = luaL_checkinteger(L, 1);
    uint32 height = luaL_checkinteger(L, 2);

    window_resize(width, height);

    return 0;
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

    widget_print(pos_x, pos_y, DRAW_COLOR, scale, "%s", str);
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

int lua_widget_theme(lua_State* L)
{
    pal_theme_t theme = (pal_theme_t)luaL_checkinteger(L, 1);

    palette_set_theme(theme);
    return 0;
}

int lua_oscilloscope_buffer_create(lua_State* L)
{
    uint32 min_value = luaL_checkinteger(L, 1);
    uint32 max_value = luaL_checkinteger(L, 2);
    uint32 size = luaL_optinteger(L, 3, 1024);
    int buffer_id = -1;
    int i;

    for (i = 0; i < MAX_OSCILLOSCOPE_BUFFERS; i++)
    {
        if (oscilloscope_buffers[i] == NULL)
        {
            oscilloscope_buffers[i] = oscilloscope_buffer_create(min_value, max_value, size);
            if (oscilloscope_buffers[i])
            {
                buffer_id = i;
            }
            break;
        }
    }

    lua_pushinteger(L, buffer_id);
    return 1;
}

int lua_oscilloscope_buffer_destroy(lua_State* L)
{
    int buffer_id = luaL_checkinteger(L, 1);

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (oscilloscope_buffers[buffer_id])
        {
            oscilloscope_buffer_destroy(oscilloscope_buffers[buffer_id]);
            oscilloscope_buffers[buffer_id] = NULL;
        }
    }
    return 0;
}

int lua_oscilloscope_buffer_push(lua_State* L)
{
    int buffer_id = luaL_checkinteger(L, 1);
    uint32 value = luaL_checkinteger(L, 2);

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (oscilloscope_buffers[buffer_id])
        {
            oscilloscope_buffer_push(oscilloscope_buffers[buffer_id], value);
        }
    }
    return 0;
}

int lua_widget_oscilloscope(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 width = luaL_checkinteger(L, 3);
    uint32 height = luaL_checkinteger(L, 4);
    int buffer_id = luaL_checkinteger(L, 5);
    uint32 current_value = luaL_checkinteger(L, 6);
    const char* label = luaL_optstring(L, 7, "OSC");
    uint64 time_window_ms = luaL_optinteger(L, 8, 0);

    if (buffer_id >= 0 && buffer_id < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (oscilloscope_buffers[buffer_id])
        {
            widget_oscilloscope(pos_x, pos_y, width, height, oscilloscope_buffers[buffer_id], current_value, label, time_window_ms);
        }
    }
    return 0;
}

int lua_widget_oscilloscope_2ch(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 width = luaL_checkinteger(L, 3);
    uint32 height = luaL_checkinteger(L, 4);
    int buffer_id1 = luaL_checkinteger(L, 5);
    uint32 current_value1 = luaL_checkinteger(L, 6);
    const char* label1 = luaL_optstring(L, 7, "OSC1");
    int buffer_id2 = luaL_checkinteger(L, 8);
    uint32 current_value2 = luaL_checkinteger(L, 9);
    const char* label2 = luaL_optstring(L, 10, "OSC2");
    uint64 time_window_ms = luaL_optinteger(L, 11, 0);

    if (buffer_id1 >= 0 && buffer_id1 < MAX_OSCILLOSCOPE_BUFFERS &&
        buffer_id2 >= 0 && buffer_id2 < MAX_OSCILLOSCOPE_BUFFERS)
    {
        if (oscilloscope_buffers[buffer_id1] && oscilloscope_buffers[buffer_id2])
        {
            widget_oscilloscope_2ch(pos_x, pos_y, width, height,
                                    oscilloscope_buffers[buffer_id1], current_value1, label1,
                                    oscilloscope_buffers[buffer_id2], current_value2, label2,
                                    time_window_ms);
        }
    }
    return 0;
}

/* Global Lua callback storage for input widgets */
#define MAX_LUA_INPUT_CALLBACKS 32
static int lua_input_callbacks[MAX_LUA_INPUT_CALLBACKS] = {LUA_NOREF};
static lua_State* lua_input_state = NULL;

/* C callback wrapper for Lua input widget */
static void lua_input_callback_wrapper(uint32 input_id, const char* text)
{
    if (input_id >= MAX_LUA_INPUT_CALLBACKS || !lua_input_state || !text)
    {
        return;
    }

    if (lua_input_callbacks[input_id] != LUA_NOREF)
    {
        lua_rawgeti(lua_input_state, LUA_REGISTRYINDEX, lua_input_callbacks[input_id]);
        lua_pushinteger(lua_input_state, input_id);
        lua_pushstring(lua_input_state, text);
        lua_call(lua_input_state, 2, 0);
    }
}

int lua_widget_input_register(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 width = luaL_checkinteger(L, 3);
    uint32 height = luaL_checkinteger(L, 4);
    const char* initial_text = luaL_optstring(L, 5, "");

    uint32 input_id = widget_input_register(pos_x, pos_y, width, height, initial_text);
    lua_pushinteger(L, input_id);
    return 1;
}

int lua_widget_input(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 width = luaL_checkinteger(L, 3);
    uint32 height = luaL_checkinteger(L, 4);
    const char* text = luaL_checkstring(L, 5);
    bool is_active = false;

    if (lua_isboolean(L, 6))
    {
        is_active = (bool)lua_toboolean(L, 6);
    }

    widget_input(pos_x, pos_y, width, height, text, is_active);
    return 0;
}

int lua_widget_input_set_callback(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);

    if (input_id >= MAX_LUA_INPUT_CALLBACKS)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    /* Store the Lua callback in the registry */
    lua_input_state = L;

    if (lua_isfunction(L, 2))
    {
        /* Remove old callback if it exists */
        if (lua_input_callbacks[input_id] != LUA_NOREF)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, lua_input_callbacks[input_id]);
        }

        /* Store new callback */
        lua_pushvalue(L, 2);
        lua_input_callbacks[input_id] = luaL_ref(L, LUA_REGISTRYINDEX);

        /* Set the C callback wrapper */
        widget_input_set_callback(input_id, lua_input_callback_wrapper);
        lua_pushboolean(L, true);
    }
    else
    {
        lua_pushboolean(L, false);
    }
    return 1;
}

int lua_widget_input_get_text(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);
    const char* text = widget_input_get_text(input_id);
    lua_pushstring(L, text);
    return 1;
}

int lua_widget_input_is_active(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);
    bool is_active = widget_input_is_active(input_id);
    lua_pushboolean(L, is_active);
    return 1;
}

int lua_widget_input_set_text(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);
    const char* text = luaL_checkstring(L, 2);

    widget_input_set_text(input_id, text);
    return 0;
}

int lua_widget_input_unregister(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);

    widget_input_unregister(input_id);
    return 0;
}

int lua_widget_input_set_position(lua_State* L)
{
    uint32 input_id = luaL_checkinteger(L, 1);
    uint32 pos_x = luaL_checkinteger(L, 2);
    uint32 pos_y = luaL_checkinteger(L, 3);

    widget_input_set_position(input_id, pos_x, pos_y);
    return 0;
}

/* Global Lua callback storage for toggle widgets */
#define MAX_LUA_TOGGLE_CALLBACKS 32
static int g_lua_toggle_callbacks[MAX_LUA_TOGGLE_CALLBACKS] = {LUA_NOREF};
static lua_State* g_lua_toggle_state = NULL;

/* C callback wrapper for Lua */
static void lua_toggle_callback_wrapper(uint32 toggle_id, bool state)
{
    if (toggle_id >= MAX_LUA_TOGGLE_CALLBACKS || ! g_lua_toggle_state)
    {
        return;
    }

    if (g_lua_toggle_callbacks[toggle_id] != LUA_NOREF)
    {
        lua_rawgeti(g_lua_toggle_state, LUA_REGISTRYINDEX, g_lua_toggle_callbacks[toggle_id]);
        lua_pushinteger(g_lua_toggle_state, toggle_id);
        lua_pushboolean(g_lua_toggle_state, state);
        lua_call(g_lua_toggle_state, 2, 0);
    }
}

int lua_widget_toggle_register(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 size = luaL_checkinteger(L, 3);
    bool initial_state = false;

    if (lua_isboolean(L, 4))
    {
        initial_state = (bool)lua_toboolean(L, 4);
    }

    uint32 toggle_id = widget_toggle_register(pos_x, pos_y, size, initial_state);
    lua_pushinteger(L, toggle_id);
    return 1;
}

int lua_widget_toggle(lua_State* L)
{
    uint32 pos_x = luaL_checkinteger(L, 1);
    uint32 pos_y = luaL_checkinteger(L, 2);
    uint32 size = luaL_checkinteger(L, 3);
    bool state = false;

    if (lua_isboolean(L, 4))
    {
        state = (bool)lua_toboolean(L, 4);
    }

    widget_toggle(pos_x, pos_y, size, state);
    return 0;
}

int lua_widget_toggle_set_callback(lua_State* L)
{
    uint32 toggle_id = luaL_checkinteger(L, 1);

    if (toggle_id >= MAX_LUA_TOGGLE_CALLBACKS)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    /* Store the Lua callback in the registry */
    g_lua_toggle_state = L;

    if (lua_isfunction(L, 2))
    {
        /* Remove old callback if it exists */
        if (g_lua_toggle_callbacks[toggle_id] != LUA_NOREF)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, g_lua_toggle_callbacks[toggle_id]);
        }

        /* Store new callback */
        lua_pushvalue(L, 2);
        g_lua_toggle_callbacks[toggle_id] = luaL_ref(L, LUA_REGISTRYINDEX);

        /* Set the C callback wrapper */
        widget_toggle_set_callback(toggle_id, lua_toggle_callback_wrapper);
        lua_pushboolean(L, true);
    }
    else
    {
        lua_pushboolean(L, false);
    }
    return 1;
}

int lua_widget_toggle_get_state(lua_State* L)
{
    uint32 toggle_id = luaL_checkinteger(L, 1);
    bool state = widget_toggle_get_state(toggle_id);
    lua_pushboolean(L, state);
    return 1;
}

int lua_widget_toggle_set_state(lua_State* L)
{
    uint32 toggle_id = luaL_checkinteger(L, 1);
    bool state = false;

    if (lua_isboolean(L, 2))
    {
        state = (bool)lua_toboolean(L, 2);
    }

    widget_toggle_set_state(toggle_id, state);
    return 0;
}

int lua_widget_toggle_set_position(lua_State* L)
{
    uint32 toggle_id = luaL_checkinteger(L, 1);
    uint32 pos_x = (uint32)luaL_checkinteger(L, 2);
    uint32 pos_y = (uint32)luaL_checkinteger(L, 3);

    widget_toggle_set_position(toggle_id, pos_x, pos_y);
    return 0;
}

int lua_widget_toggle_unregister(lua_State* L)
{
    uint32 toggle_id = luaL_checkinteger(L, 1);

    if (toggle_id < MAX_LUA_TOGGLE_CALLBACKS && g_lua_toggle_callbacks[toggle_id] != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, g_lua_toggle_callbacks[toggle_id]);
        g_lua_toggle_callbacks[toggle_id] = LUA_NOREF;
    }

    widget_toggle_unregister(toggle_id);
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
    lua_pushcfunction(core->L, lua_window_resize);
    lua_setglobal(core->L, "window_resize");
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
    lua_pushcfunction(core->L, lua_widget_theme);
    lua_setglobal(core->L, "widget_theme");
    lua_pushcfunction(core->L, lua_widget_input_register);
    lua_setglobal(core->L, "widget_input_register");
    lua_pushcfunction(core->L, lua_widget_input);
    lua_setglobal(core->L, "widget_input");
    lua_pushcfunction(core->L, lua_widget_input_set_callback);
    lua_setglobal(core->L, "widget_input_set_callback");
    lua_pushcfunction(core->L, lua_widget_input_get_text);
    lua_setglobal(core->L, "widget_input_get_text");
    lua_pushcfunction(core->L, lua_widget_input_is_active);
    lua_setglobal(core->L, "widget_input_is_active");
    lua_pushcfunction(core->L, lua_widget_input_set_text);
    lua_setglobal(core->L, "widget_input_set_text");
    lua_pushcfunction(core->L, lua_widget_input_set_position);
    lua_setglobal(core->L, "widget_input_set_position");
    lua_pushcfunction(core->L, lua_widget_input_unregister);
    lua_setglobal(core->L, "widget_input_unregister");
    lua_pushcfunction(core->L, lua_widget_toggle_register);
    lua_setglobal(core->L, "widget_toggle_register");
    lua_pushcfunction(core->L, lua_widget_toggle);
    lua_setglobal(core->L, "widget_toggle");
    lua_pushcfunction(core->L, lua_widget_toggle_set_callback);
    lua_setglobal(core->L, "widget_toggle_set_callback");
    lua_pushcfunction(core->L, lua_widget_toggle_get_state);
    lua_setglobal(core->L, "widget_toggle_get_state");
    lua_pushcfunction(core->L, lua_widget_toggle_set_state);
    lua_setglobal(core->L, "widget_toggle_set_state");
    lua_pushcfunction(core->L, lua_widget_toggle_set_position);
    lua_setglobal(core->L, "widget_toggle_set_position");
    lua_pushcfunction(core->L, lua_widget_toggle_unregister);
    lua_setglobal(core->L, "widget_toggle_unregister");
    lua_pushcfunction(core->L, lua_oscilloscope_buffer_create);
    lua_setglobal(core->L, "oscilloscope_buffer_create");
    lua_pushcfunction(core->L, lua_oscilloscope_buffer_destroy);
    lua_setglobal(core->L, "oscilloscope_buffer_destroy");
    lua_pushcfunction(core->L, lua_oscilloscope_buffer_push);
    lua_setglobal(core->L, "oscilloscope_buffer_push");
    lua_pushcfunction(core->L, lua_widget_oscilloscope);
    lua_setglobal(core->L, "widget_oscilloscope");
    lua_pushcfunction(core->L, lua_widget_oscilloscope_2ch);
    lua_setglobal(core->L, "widget_oscilloscope_2ch");
}
