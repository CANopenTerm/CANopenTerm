/** @file lua_widget.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_WIDGET_H
#define LUA_WIDGET_H

#include "core.h"
#include "lua.h"

int lua_clear_window(lua_State* L);
int lua_window_is_shown(lua_State* L);
int lua_hide_window(lua_State* L);
int lua_show_window(lua_State* L);
int lua_widget_tachometer(lua_State* L);
void lua_register_widget_commands(core_t* core);

#endif /* LUA_WIDGET_H */
