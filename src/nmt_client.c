/** @file nmt_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "can.h"
#include "core.h"
#include "nmt_client.h"
#include "printf.h"
#include "table.h"

Uint32 nmt_send_command(Uint8 node_id, nmt_command_t command, SDL_bool show_output)
{
    Uint32        can_status  = 0;
    can_message_t can_message = { 0 };

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    can_message.id      = 0x000;
    can_message.length  = 2;
    can_message.data[0] = command;
    can_message.data[1] = node_id;

    switch (command)
    {
        case NMT_OPERATIONAL:
        case NMT_STOP:
        case NMT_PRE_OPERATIONAL:
        case NMT_RESET_NODE:
        case NMT_RESET_COMM:
            break;
        default:
            nmt_print_help();
            return can_status;
    }

    can_status = can_write(&can_message);
    if (0 != can_status)
    {
        can_print_error_message(NULL, can_status, show_output);
    }

    return can_status;
}

int lua_nmt_send_command(lua_State* L)
{
    int node_id = luaL_checkinteger(L, 1);
    int command = luaL_checkinteger(L, 2);

    switch (command)
    {
        case NMT_OPERATIONAL:
        case NMT_STOP:
        case NMT_PRE_OPERATIONAL:
        case NMT_RESET_NODE:
        case NMT_RESET_COMM:
            break;
        default:
            return 0;
    }

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (0 == nmt_send_command(node_id, command, SDL_FALSE))
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}

void lua_register_nmt_command(core_t* core)
{
    lua_pushcfunction(core->L, lua_nmt_send_command);
    lua_setglobal(core->L, "nmt_send_command");
}

void nmt_print_help(void)
{
    table_t table = { DARK_CYAN, DARK_WHITE, 4, 5, 30 };

    table_print_header(&table);
    table_print_row("CMD", "Alias", "Description", &table);
    table_print_divider(&table);
    table_print_row("0x01", "op",    "Start (go to Operational)",      &table);
    table_print_row("0x02", "stop",  "Stop (go to Stopped)",           &table);
    table_print_row("0x80", "preop", "Go to Pre-operational",          &table);
    table_print_row("0x81", "reset", "Reset node (Application reset)", &table);
    table_print_row("0x82", " ",     "Reset communication",            &table);
    table_print_footer(&table);
}
