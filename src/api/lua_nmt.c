/** @file lua_nmt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua_nmt.h"
#include "can.h"
#include "core.h"
#include "lauxlib.h"
#include "lua.h"
#include "nmt.h"
#include "os.h"

extern void nmt_print_error(const char* reason, nmt_command_t command, disp_mode_t disp_mode);

int lua_nmt_send_command(lua_State* L)
{
    uint32      status;
    disp_mode_t disp_mode   = SILENT;
    int         node_id     = luaL_checkinteger(L, 1);
    int         command     = luaL_checkinteger(L, 2);
    bool      show_output = lua_toboolean(L, 3);
    const char* comment     = lua_tostring(L, 4);

    limit_node_id((uint8*)&node_id);

    if (true == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    switch (command)
    {
        case NMT_OPERATIONAL:
        case NMT_STOP:
        case NMT_PRE_OPERATIONAL:
        case NMT_RESET_NODE:
        case NMT_RESET_COMM:
            status = nmt_send_command(node_id, command, disp_mode, comment);
            if (0 == status)
            {
                lua_pushboolean(L, 1);
            }
            else
            {
                nmt_print_error(can_get_error_message(status), command, disp_mode);
                lua_pushboolean(L, 0);
            }
            break;
        default:
            nmt_print_error("Unknown NMT command", command, disp_mode);
            lua_pushboolean(L, 0);
            break;
    }

    return 1;
}

void lua_register_nmt_command(core_t* core)
{
    lua_pushcfunction(core->L, lua_nmt_send_command);
    lua_setglobal(core->L, "nmt_send_command");
}
