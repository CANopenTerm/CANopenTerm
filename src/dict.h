/** @file dict.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef DICT_H
#define DICT_H

#include "SDL.h"
#include "core.h"
#include "lua.h"

typedef struct dict_entry
{
    Uint16      index_start;
    Uint16      index_end;
    Uint8       sub_index_start;
    Uint8       sub_index_end;
    const char* description;

} dict_entry_t;

const char* dict_lookup(Uint16 index, Uint8 sub_index);
int         lua_dict_lookup(lua_State* L);
void        lua_register_dict_commands(core_t* core);

#endif /* DICT_H */