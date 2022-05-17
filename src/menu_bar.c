/** @file menu_bar.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "can.h"
#include "core.h"
#include "menu_bar.h"
#include "nuklear.h"

void menu_bar_widget(core_t* core)
{
    if (nk_begin(
            core->ctx,
            "Menu",
            nk_rect(0, 0, WINDOW_WIDTH, 30),
            NK_WINDOW_NO_SCROLLBAR |
            NK_WINDOW_BACKGROUND))
    {
        nk_menubar_begin(core->ctx);
        nk_layout_row_begin(core->ctx, NK_STATIC, 25, 5);
        nk_layout_row_push(core->ctx, 45);

        if (nk_menu_begin_label(core->ctx, "Menu", NK_TEXT_LEFT, nk_vec2(150, 200)))
        {
            char node_id[5] = { 0 };
            SDL_snprintf(node_id, 5, "0x%2x", core->node_id);
            if (core->node_id <= 0xf)
            {
                node_id[2] = '0';
            }

            nk_layout_row_dynamic(core->ctx, 20, 1);
            if (SDL_TRUE == is_can_initialised(core))
            {
                nk_text_colored(core->ctx, "Connected", 9, NK_TEXT_LEFT, nk_rgba(0x00, 0xcc, 0x00, 0xff));
            }
            else
            {
                nk_text_colored(core->ctx, "Disconnected", 12, NK_TEXT_LEFT, nk_rgba(0xcc, 0x00, 0x00, 0xff));
            }

            nk_layout_row_dynamic(core->ctx, 20, 1);
            nk_label(core->ctx, "Node ID", NK_TEXT_LEFT);
            nk_layout_row_dynamic(core->ctx, 20, 2);
            nk_slider_int(core->ctx, 0x00, (int*)&core->node_id, 0x7f, 1);
            nk_label(core->ctx, node_id, NK_TEXT_LEFT);
            if (nk_menu_item_label(core->ctx, "Quit", NK_TEXT_LEFT))
            {
                core->is_running = SDL_FALSE;
            }
            nk_menu_end(core->ctx);
        }
        nk_menubar_end(core->ctx);
    }
    nk_end(core->ctx);
}
