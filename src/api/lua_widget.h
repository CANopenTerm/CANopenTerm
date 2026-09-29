/** @file lua_widget.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LUA_WIDGET_H
#define LUA_WIDGET_H

#include "core.h"
#include "lua.h"

int lua_clear_window(lua_State* L);
int lua_window_is_shown(lua_State* L);
int lua_window_hide(lua_State* L);
int lua_window_resize(lua_State* L);
int lua_window_show(lua_State* L);
int lua_widget_tachometer(lua_State* L);
int lua_widget_theme(lua_State* L);
void lua_register_widget_commands(core_t* core);

#endif /* LUA_WIDGET_H */
