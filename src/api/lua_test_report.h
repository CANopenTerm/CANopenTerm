/** @file lua_test_report.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_TEST_REPORT_H
#define LUA_TEST_REPORT_H

#include "core.h"
#include "lua.h"

int lua_test_add_result(lua_State* L);
int lua_test_clear_results(lua_State* L);
int lua_test_eds_file(lua_State* L);
int lua_test_generate_report(lua_State* L);
void lua_register_test_commands(core_t* core);

#endif /* LUA_TEST_REPORT_H */
