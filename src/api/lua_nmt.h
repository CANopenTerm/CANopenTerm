/** @file lua_nmt.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_NMT_H
#define LUA_NMT_H

#include "core.h"
#include "lua.h"

int  lua_nmt_send_command(lua_State *L);
void lua_register_nmt_command(core_t *core);

#endif /* LUA_NMT_H */
