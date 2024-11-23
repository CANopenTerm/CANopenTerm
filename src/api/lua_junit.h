/** @file lua_junit.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_JUNIT_H
#define LUA_JUNIT_H

#include "core.h"
#include "lua.h"

int  lua_junit_add_result(lua_State* L);
int  lua_junit_clear_results(lua_State* L);
int  lua_junit_generate_report(lua_State* L);
void lua_register_junit_commands(core_t* core);

#endif /* LUA_JUNIT_H */
