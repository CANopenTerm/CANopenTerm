/** @file nmt_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef _WIN32
#include <windows.h>
#endif
#include "PCANBasic.h"

#include "SDL.h"
#include "core.h"
#include "nmt_client.h"
#include "nuklear.h"

Uint32 send_nmt_command(nmt_command_t command, Uint8 node_id)
{
    Uint32   can_status;
    TPCANMsg can_message;

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    can_message.ID      = 0x000;
    can_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    can_message.LEN     = 2;
    can_message.DATA[0] = command;
    can_message.DATA[1] = node_id;

    can_status = CAN_Write(PCAN_USBBUS1, &can_message);
    if (PCAN_ERROR_OK != can_status)
    {
        char err_message[100] = { 0 };
        CAN_GetErrorText(can_status, 0x09, err_message);
        SDL_LogWarn(0, "Could not send NMT command 0x%x: %s", command, err_message);
    }

    return can_status;
}

void nmt_client_widget(core_t* core)
{
    int window_width;
    int window_height;

    if (NULL == core)
    {
        return;
    }

    SDL_GetWindowSize(core->window, &window_width, &window_height);

    if (0 != nk_begin(
            core->ctx,
            "NMT commands",
            nk_rect((float)window_width - 220, 10, 210, 180),
            NK_WINDOW_BORDER  |
            NK_WINDOW_TITLE   |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_begin(core->ctx, NK_STATIC, 25, 1);
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x01 Enter Operational    ", 26))
        {
            core->can_status = send_nmt_command(NMT_OPERATIONAL, core->node_id);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x02 Enter Stop           ", 26))
        {
            core->can_status = send_nmt_command(NMT_STOP, core->node_id);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x80 Enter Pre-operational", 26))
        {
            core->can_status = send_nmt_command(NMT_PRE_OPERATIONAL, core->node_id);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x81 Reset node           ", 26))
        {
            core->can_status = send_nmt_command(NMT_RESET_NODE, core->node_id);
        }
        nk_layout_row_push(core->ctx, 195);
        if (0 != nk_button_text(core->ctx, "0x82 Reset communication  ", 26))
        {
            core->can_status = send_nmt_command(NMT_RESET_COMM, core->node_id);
        }
        nk_layout_row_end(core->ctx);
    }

    nk_end(core->ctx);
}
