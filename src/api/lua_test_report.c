/** @file lua_test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "eds.h"
#include "lauxlib.h"
#include "lua.h"
#include "os.h"
#include "test_report.h"

int lua_test_add_result(lua_State* L)
{
    test_result_t result;

    result.has_passed    = lua_toboolean(L, 1);
    result.time          = lua_tonumber(L, 2);
    result.package       = lua_tostring(L, 3);
    result.class_name    = lua_tostring(L, 4);
    result.test_name     = lua_tostring(L, 5);
    result.error_type    = lua_tostring(L, 6);
    result.error_message = lua_tostring(L, 7);
    result.call_stack    = lua_tostring(L, 8);

    test_add_result(&result);

    return 0;
}

int lua_test_clear_results(lua_State* L)
{
    test_clear_results();
    return 0;
}

int lua_test_eds_file(lua_State* L)
{
    int         node_id   = luaL_checkinteger(L, 1);
    const char* file_name = lua_tostring(L, 2);
    const char* package   = lua_tostring(L, 3);

    if (NULL == package)
    {
        package = "EDS";
    }

    run_conformance_test(file_name, package, node_id, SCRIPT_MODE);

    return 0;
}

int lua_test_generate_report(lua_State* L)
{
    const char* file_name = lua_tostring(L, 1);
    status_t    status    = test_generate_report(file_name);

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

void lua_register_test_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_test_add_result);
    lua_setglobal(core->L, "test_add_result");
    lua_pushcfunction(core->L, lua_test_clear_results);
    lua_setglobal(core->L, "test_clear_results");
    lua_pushcfunction(core->L, lua_test_eds_file);
    lua_setglobal(core->L, "test_eds_file");
    lua_pushcfunction(core->L, lua_test_generate_report);
    lua_setglobal(core->L, "test_generate_report");
}
