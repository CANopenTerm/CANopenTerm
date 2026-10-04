/** @file lua_misc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua_misc.h"
#include "command.h"
#include "core.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"
#include "scripts.h"

int lua_delay_ms(lua_State* L)
{
    uint32 delay_in_ms = (uint32)lua_tointeger(L, 1);
    bool show_output = lua_toboolean(L, 2);
    const char* comment = lua_tostring(L, 3);

    if (0 == delay_in_ms)
    {
        delay_in_ms = 1u;
    }

    if (true == show_output)
    {
        int i;
        char buffer[34] = {0};

        os_print(LIGHT_BLACK, "Delay ");
        os_print(DEFAULT_COLOR, "   -       -       -         -       -       ");

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        os_print(DARK_MAGENTA, "%s ", buffer);
        os_print(DEFAULT_COLOR, "%ums\n", delay_in_ms);
    }

    os_delay(delay_in_ms);
    return 1;
}

int lua_console_hide(lua_State* L)
{
    os_console_hide();
    return 0;
}

int lua_console_show(lua_State* L)
{
    os_console_show();
    return 0;
}

int lua_key_is_hit(lua_State* L)
{
    lua_pushboolean(L, os_key_is_hit());
    return 1;
}

int lua_key_send(lua_State* L)
{
    uint16 key = (uint16)lua_tointeger(L, 1);
    os_key_send(key);
    return 0;
}

int lua_print_heading(lua_State* L)
{
    const char* heading = lua_tostring(L, 1);

    print_heading(heading);

    return 0;
}

int lua_print_result(lua_State* L)
{
    uint8 id = (uint8)lua_tointeger(L, 1);
    uint16 index = (uint16)lua_tointeger(L, 2);
    uint8 sub_index = (uint8)lua_tointeger(L, 3);
    uint32 length = (uint32)lua_tointeger(L, 4);
    bool success = lua_toboolean(L, 5);
    const char* comment = lua_tostring(L, 6);
    uint32_t data = (uint32_t)lua_tointeger(L, 7);

    print_result(id, index, sub_index, length, success, comment, data);

    return 0;
}

int lua_run_sequence(lua_State* L)
{
    int arg_count = lua_gettop(L);
    core_t* core = NULL;
    int i;

    if (arg_count < 1)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    /* Get core pointer from Lua registry */
    lua_getfield(L, LUA_REGISTRYINDEX, "core_ptr");
    if (! lua_islightuserdata(L, -1))
    {
        lua_pushboolean(L, false);
        return 1;
    }
    core = (core_t*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (NULL == core)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    /* Execute each command in the sequence */
    for (i = 1; i <= arg_count; ++i)
    {
        if (lua_isstring(L, i))
        {
            char buffer[512];
            const char* cmd = lua_tostring(L, i);

            os_strlcpy(buffer, cmd, sizeof(buffer));
            parse_command(buffer, core, SCRIPT_MODE);
        }
    }

    lua_pushboolean(L, true);
    return 1;
}

void lua_register_misc_commands(core_t* core)
{
    /* Store core pointer in Lua registry for use by lua_run_sequence. */
    lua_pushlightuserdata(core->L, (void*)core);
    lua_setfield(core->L, LUA_REGISTRYINDEX, "core_ptr");

    lua_pushcfunction(core->L, lua_delay_ms);
    lua_setglobal(core->L, "delay_ms");

    lua_pushcfunction(core->L, lua_console_hide);
    lua_setglobal(core->L, "console_hide");

    lua_pushcfunction(core->L, lua_console_show);
    lua_setglobal(core->L, "console_show");

    lua_pushcfunction(core->L, lua_key_is_hit);
    lua_setglobal(core->L, "key_is_hit");

    lua_pushcfunction(core->L, lua_key_send);
    lua_setglobal(core->L, "key_send");

    lua_pushcfunction(core->L, lua_print_heading);
    lua_setglobal(core->L, "print_heading");

    lua_pushcfunction(core->L, lua_print_result);
    lua_setglobal(core->L, "print_result");

    lua_pushcfunction(core->L, lua_run_sequence);
    lua_setglobal(core->L, "run_sequence");
}
