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

#include "lua.h"

#define PDO_MAX 0x1f8 /* TPDO1 - TPDO4 */

typedef struct pdo
{
    os_timer_id id;
    uint16      can_id;
    uint8       length;
    uint64      data;

} pdo_t;

bool_t pdo_add(uint16 can_id, uint32 event_time_ms, uint8 length, uint64 data, disp_mode_t disp_mode);
bool_t pdo_del(uint16 can_id, disp_mode_t disp_mode);
int    lua_pdo_add(lua_State* L);
int    lua_pdo_del(lua_State* L);
void   lua_register_pdo_commands(core_t* core);
void   pdo_print_help(void);
bool_t pdo_is_id_valid(uint16 can_id);

#endif /* PDO_H */
