/** @file lua_junit.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "junit.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"

int lua_junit_add_result(lua_State* L)
{
    junit_result_t result;

    result.has_passed    = lua_toboolean(L, 1);
    result.time          = lua_tonumber(L, 2);
    result.package       = lua_tostring(L, 3);
    result.class_name    = lua_tostring(L, 4);
    result.test_name     = lua_tostring(L, 5);
    result.error_type    = lua_tostring(L, 6);
    result.error_message = lua_tostring(L, 7);
    result.call_stack    = lua_tostring(L, 8);

    junit_add_result(&result);

    return 0;
}

int lua_junit_clear_results(lua_State* L)
{
    junit_clear_results();
    return 0;
}

int lua_junit_generate_report(lua_State* L)
{
    const char* filename = lua_tostring(L, 1);
    status_t    status   = junit_generate_report(filename);

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

void lua_register_junit_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_junit_add_result);
    lua_setglobal(core->L, "junit_add_result");
    lua_pushcfunction(core->L, lua_junit_clear_results);
    lua_setglobal(core->L, "junit_clear_results");
    lua_pushcfunction(core->L, lua_junit_generate_report);
    lua_setglobal(core->L, "junit_generate_report");
}
