/** @file core.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CORE_H
#define CORE_H

#include "lua.h"
#include "os.h"

typedef enum disp_mode
{
    SILENT = 0,
    TERM_MODE,
    SCRIPT_MODE

} disp_mode_t;

typedef struct core
{
    os_thread* can_monitor_th;
    lua_State* L;
    uint8      baud_rate;
    uint32     can_channel;
    uint32     can_status;
    uint8      node_id;
    bool_t     is_can_initialised;
    bool_t     is_running;
    bool_t     is_script_running;
    bool_t     is_plain_mode;
    char       can_interface[32];

} core_t;

extern core_t* core;

status_t core_init(core_t** core, bool_t is_plain_mode);
void     core_register_ctrl_c_handler(void);
status_t core_update(core_t* core);
void     core_deinit(core_t* core);

#endif /* CORE_H */
