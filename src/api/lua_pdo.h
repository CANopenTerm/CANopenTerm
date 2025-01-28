/** @file lua_pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_PDO_H
#define LUA_PDO_H

#include "core.h"
#include "lua.h"

int  lua_pdo_add(lua_State* L);
int  lua_pdo_del(lua_State* L);
void lua_register_pdo_commands(core_t* core);

#endif /* LUA_PDO_H */
