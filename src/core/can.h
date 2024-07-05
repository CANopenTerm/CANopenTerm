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

#include "lua.h"
#include "core.h"
#include "os.h"

typedef struct can_message
{
    uint32 id;
    uint32 length;
    uint8  data[CAN_MAX_DATA_LENGTH + 1];
    uint64 timestamp_us;

} can_message_t;

void        can_init(core_t* core_t);
void        can_deinit(core_t* core);
const char* can_get_error_message(uint32 can_status);
void        can_quit(core_t* core);
uint32      can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment);
uint32      can_read(can_message_t* message);
status_t    can_print_baud_rate_help(core_t* core);
status_t    can_print_channel_help(core_t* core);
void        can_print_error(uint32 can_id, const char* reason, disp_mode_t disp_mode);
void        can_set_baud_rate(uint8 baud_rate_index, core_t* core);
void        can_set_channel(int channel, core_t* core);
void        limit_node_id(uint8* node_id);
int         lua_can_write(lua_State* L);
int         lua_can_read(lua_State* L);
void        lua_register_can_commands(core_t* core);
bool_t      is_can_initialised(core_t* core);

#endif /* CAN_H */
