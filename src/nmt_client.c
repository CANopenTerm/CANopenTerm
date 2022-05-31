/** @file nmt_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "can.h"
#include "core.h"
#include "nmt_client.h"
#include "nuklear.h"
#include "printf.h"
#include "table.h"

Uint32 nmt_send_command(Uint8 node_id, nmt_command_t command)
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
        can_print_error_message(NULL, can_status);
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

    if (0 != nmt_send_command(node_id, command))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void lua_register_nmt_command(core_t* core)
{
    lua_pushcfunction(core->L, lua_nmt_send_command);
    lua_setglobal(core->L, "nmt_send_command");
}

void nmt_client_widget(core_t* core)
{
    int window_width;
    int window_height;

    if (NULL == core)
    {
        return;
    }

    if (SDL_FALSE == core->is_gui_active)
    {
        return;
    }

    SDL_GetWindowSize(core->window, &window_width, &window_height);

    if (0 != nk_begin(
            core->ctx,
            "NMT commands",
            nk_rect((float)window_width - 220, 40, 210, 180),
            NK_WINDOW_BORDER  |
            NK_WINDOW_TITLE   |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_begin(core->ctx, NK_STATIC, 25, 1);
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x01 Enter Operational    ", 26))
        {
            core->can_status = nmt_send_command(core->node_id, NMT_OPERATIONAL);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x02 Enter Stop           ", 26))
        {
            core->can_status = nmt_send_command(core->node_id, NMT_STOP);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x80 Enter Pre-operational", 26))
        {
            core->can_status = nmt_send_command(core->node_id, NMT_PRE_OPERATIONAL);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x81 Reset node           ", 26))
        {
            core->can_status = nmt_send_command(core->node_id, NMT_RESET_NODE);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x82 Reset communication  ", 26))
        {
            core->can_status = nmt_send_command(core->node_id, NMT_RESET_COMM);
        }
        nk_layout_row_end(core->ctx);
    }

    nk_end(core->ctx);
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
