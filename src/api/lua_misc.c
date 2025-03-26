/** @file lua_misc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua_misc.h"
#include "core.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"
#include "scripts.h"

extern void print_heading(const char* heading);

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

int lua_key_is_hit(lua_State* L)
{
    lua_pushboolean(L, os_key_is_hit());
    return 1;
}

int lua_print_heading(lua_State* L)
{
    const char* heading = lua_tostring(L, 1);

    print_heading(heading);

    return 0;
}

void lua_register_misc_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_delay_ms);
    lua_setglobal(core->L, "delay_ms");

    lua_pushcfunction(core->L, lua_key_is_hit);
    lua_setglobal(core->L, "key_is_hit");

    lua_pushcfunction(core->L, lua_print_heading);
    lua_setglobal(core->L, "print_heading");
}
