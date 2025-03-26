/** @file lua_dbc.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_DBC_H
#define LUA_DBC_H

#include "core.h"
#include "lua.h"

int lua_dbc_decode(lua_State* L);
int lua_dbc_find_id_by_name(lua_State* L);
int lua_dbc_load(lua_State* L);
void lua_register_dbc_commands(core_t* core);

#endif /* LUA_DBC_H */
