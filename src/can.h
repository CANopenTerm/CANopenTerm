/** @file can.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CAN_H
#define CAN_H

#define CAN_MAX_DATA_LENGTH 0xff

#include "SDL.h"
#include "lua.h"
#include "core.h"

typedef struct can_message
{
    Uint16 id;
    Uint8  length;
    Uint8  data[CAN_MAX_DATA_LENGTH + 1];
    Uint64 timestamp_us;

} can_message_t;

void        can_init(core_t* core_t);
void        can_deinit(core_t* core);
void        can_quit(core_t* core);
Uint32      can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment);
Uint32      can_read(can_message_t* message);
void        can_set_baud_rate(Uint8 command, core_t* core);
int         lua_can_write(lua_State* L);
int         lua_can_read(lua_State* L);
void        lua_register_can_commands(core_t* core);
const char* can_get_error_message(Uint32 can_status);
void        can_print_baud_rate_help(core_t* core);
SDL_bool    is_can_initialised(core_t* core);

#endif /* CAN_H */
