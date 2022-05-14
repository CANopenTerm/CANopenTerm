/** @file nmt_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "config.h"
#include "nuklear.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "PCANBasic.h"

typedef enum
{
    NMT_OPERATIONAL     = 0x01,
    NMT_STOP            = 0x02,
    NMT_PRE_OPERATIONAL = 0x80,
    NMT_RESET_NODE      = 0x81,
    NMT_RESET_COMM      = 0x82

} nmt_command_t;

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
        SDL_LogWarn(0, "Could not send NMT command 0x%2x: %s", command, err_message);
    }

    return can_status;
}

void nmt_client_widget(struct nk_context *ctx, Uint32* can_status)
{
    Uint8 node_id = 0x30; // DEBUG

    if (0 != nk_begin(
            ctx,
            "NMT commands",
            nk_rect(WINDOW_WIDTH - 220, 10, 210, 180),
            NK_WINDOW_BORDER  |
            NK_WINDOW_TITLE   |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_begin(ctx, NK_STATIC, 25, 1);
        nk_layout_row_push(ctx, 195);
        if (0 != nk_button_text(ctx, "0x01 Enter Operational    ", 26))
        {
            *can_status = send_nmt_command(NMT_OPERATIONAL, node_id);
        }
        nk_layout_row_push(ctx, 195);
        if (0 != nk_button_text(ctx, "0x02 Enter Stop           ", 26))
        {
            *can_status = send_nmt_command(NMT_STOP, node_id);
        }
        nk_layout_row_push(ctx, 195);
        if (0 != nk_button_text(ctx, "0x80 Enter Pre-operational", 26))
        {
            *can_status = send_nmt_command(NMT_PRE_OPERATIONAL, node_id);
        }
        nk_layout_row_push(ctx, 195);
        if (0 != nk_button_text(ctx, "0x81 Reset node           ", 26))
        {
            *can_status = send_nmt_command(NMT_RESET_NODE, node_id);
        }
        nk_layout_row_push(ctx, 195);
        if (0 != nk_button_text(ctx, "0x82 Reset communication  ", 26))
        {
            *can_status = send_nmt_command(NMT_RESET_COMM, node_id);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}
