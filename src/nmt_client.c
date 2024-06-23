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

static void print_error(nmt_command_t command, Uint32 can_status, disp_mode_t disp_mode);

Uint32 nmt_send_command(Uint8 node_id, nmt_command_t command, disp_mode_t disp_mode, const char* comment)
{
    Uint32        can_status  = 0;
    can_message_t can_message = { 0 };

    limit_node_id(&node_id);

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

    can_status = can_write(&can_message, NO_OUTPUT, NULL);
    if (0 != can_status)
    {
        print_error(command, can_status, disp_mode);
    }
    else
    {
        if (SCRIPT_OUTPUT == disp_mode)
        {
            int  i;
            char buffer[34] = { 0 };

            c_printf(DARK_CYAN, "NMT  ");
            c_printf(DEFAULT_COLOR, "    0x%02X    -       -         -       ", node_id);
            c_printf(LIGHT_GREEN, "SUCC    ");

            if (NULL == comment)
            {
                comment = "-";
            }

            SDL_strlcpy(buffer, comment, 33);
            for (i = SDL_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            c_printf(DARK_MAGENTA, "%s ", buffer);

            switch (command)
            {
                case 0x01:
                    c_printf(DEFAULT_COLOR, "Start (go to Operational)\n");
                    break;
                case 0x02:
                    c_printf(DEFAULT_COLOR, "Stop (go to Stopped)\n");
                    break;
                case 0x80:
                    c_printf(DEFAULT_COLOR, "Go to Pre-operational\n");
                    break;
                case 0x81:
                    c_printf(DEFAULT_COLOR, "Reset node (Application reset)\n");
                    break;
                case 0x82:
                    c_printf(DEFAULT_COLOR, "Reset communication\n");
                    break;
            }
        }
    }

    return can_status;
}

int lua_nmt_send_command(lua_State* L)
{
    disp_mode_t disp_mode   = NO_OUTPUT;
    int         node_id     = luaL_checkinteger(L, 1);
    int         command     = luaL_checkinteger(L, 2);
    SDL_bool    show_output = lua_toboolean(L, 3);
    const char* comment     = lua_tostring(L, 4);

    limit_node_id((Uint8*)&node_id);

    if (SDL_TRUE == show_output)
    {
        disp_mode = SCRIPT_OUTPUT;
    }

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

    if (0 == nmt_send_command(node_id, command, disp_mode, comment))
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

static void print_error(nmt_command_t command, Uint32 can_status, disp_mode_t disp_mode)
{
    switch (disp_mode)
    {
        case TERM_OUTPUT:
        {        
            c_log(LOG_ERROR, "NMT 0x%02X error: %s", command, can_get_error_message(can_status));
            break;
        }
        case SCRIPT_OUTPUT:
        {
            c_printf(LIGHT_BLACK, "NMT  ");
            c_printf(DEFAULT_COLOR, "    0x%02X    -       -         -       ", command);
            c_printf(LIGHT_RED, "FAIL    ");
            c_printf(DEFAULT_COLOR, "%s\n", can_get_error_message(can_status));
            break;
        }
        default:
        case NO_OUTPUT:
            break;
    }
}
