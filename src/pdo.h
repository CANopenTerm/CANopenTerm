/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PDO_H
#define PDO_H

#include "SDL.h"
#include "lua.h"

#define PDO_MAX 0x1f8 // TPDO1 - TPDO4

typedef struct pdo
{
    SDL_TimerID id;
    Uint16      can_id;
    Uint8       length;
    Uint64      data;

} pdo_t;

void     pdo_add(Uint16 can_id, Uint32 event_time_ms, Uint8 length, Uint64 data);
void     pdo_del(Uint16 can_id);
int      lua_pdo_add(lua_State* L);
int      lua_pdo_del(lua_State* L);
void     lua_register_pdo_commands(core_t* core);
void     pdo_print_help(void);
SDL_bool pdo_is_id_valid(Uint16 can_id);

#endif /* PDO_H */
