/** @file nmt.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef NMT_H
#define NMT_H

#include "lua.h"
#include "core.h"
#include "os.h"

typedef enum
{
    NMT_OPERATIONAL     = 0x01,
    NMT_STOP            = 0x02,
    NMT_PRE_OPERATIONAL = 0x80,
    NMT_RESET_NODE      = 0x81,
    NMT_RESET_COMM      = 0x82

} nmt_command_t;

status_t nmt_send_command(uint8 node_id, nmt_command_t command, disp_mode_t disp_mode, const char* comment);
int      lua_nmt_send_command(lua_State* L);
void     lua_register_nmt_command(core_t* core);
status_t nmt_print_help(disp_mode_t disp_mode);

#endif /* NMT_H */
