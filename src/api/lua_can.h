/** @file lua_dbc.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_CAN_H
#define LUA_CAN_H

#include "core.h"
#include "lua.h"

int  lua_can_write(lua_State* L);
int  lua_can_read(lua_State* L);
int  lua_can_flush(lua_State* L);
int  lua_dict_lookup_raw(lua_State* L);
void lua_register_can_commands(core_t* core);

#endif /* LUA_CAN_H */
