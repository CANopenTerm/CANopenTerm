/** @file lua_misc.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_MISC_H
#define LUA_MISC_H

#include "core.h"
#include "lua.h"

int  lua_delay_ms(lua_State* L);
int  lua_key_is_hit(lua_State* L);
int  lua_print_heading(lua_State* L);
void lua_register_misc_commands(core_t* core);

#endif /* LUA_MISC_H */
