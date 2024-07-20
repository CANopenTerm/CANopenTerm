/** @file lua_pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_PDO_H
#define LUA_PDO_H

#include "core.h"
#include "lua.h"

int  lua_sdo_lookup_abort_code(lua_State *L);
int  lua_sdo_read(lua_State *L);
int  lua_sdo_write(lua_State *L);
int  lua_sdo_write_file(lua_State *L);
int  lua_sdo_write_string(lua_State *L);
int  lua_dict_lookup(lua_State *L);
void lua_register_sdo_commands(core_t *core);

#endif /* LUA_PDO_H */
