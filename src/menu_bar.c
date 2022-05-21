/** @file menu_bar.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "nuklear.h"
#include "can.h"
#include "core.h"
#include "menu_bar.h"

void menu_bar_widget(core_t* core)
{
    static SDL_bool show_about = SDL_FALSE;

    int window_width;
    int window_height;

    if (NULL == core)
    {
        return;
    }

    if (NULL == core)
    {
        return;
    }

    SDL_GetWindowSize(core->window, &window_width, &window_height);

    if (nk_begin(
            core->ctx,
            "Menu",
            nk_rect(0, 0, window_width, 30),
            NK_WINDOW_NO_SCROLLBAR |
            NK_WINDOW_BACKGROUND))
    {
        nk_menubar_begin(core->ctx);
        nk_layout_row_begin(core->ctx, NK_STATIC, 25, 5);
        nk_layout_row_push(core->ctx, 55);

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
            nk_layout_row_dynamic(core->ctx, 20, 1);
            if (nk_menu_item_label(core->ctx, "About", NK_TEXT_LEFT))
            {
                show_about = SDL_TRUE;
            }
            if (nk_menu_item_label(core->ctx, "Quit", NK_TEXT_LEFT))
            {
                gui_deinit(core);
            }
            nk_menu_end(core->ctx);
        }

        if (nk_menu_begin_label(core->ctx, "Scripts", NK_TEXT_LEFT, nk_vec2(150, 200)))
        {
            DIR *dir;
            nk_layout_row_dynamic(core->ctx, 20, 1);

            dir = opendir("./scripts");
            if (NULL != dir)
            {
                struct dirent *ent;
                while ((ent = readdir (dir)) != NULL)
                {
                    switch (ent->d_type)
                    {
                        case DT_REG:
                            if (nk_menu_item_label(core->ctx, ent->d_name, NK_TEXT_LEFT))
                            {
                                char script_path[64] = { 0 };
                                SDL_snprintf(script_path, 64, "scripts/%s", ent->d_name);
                                if (LUA_OK == luaL_dofile(core->L, script_path))
                                {
                                    lua_pop(core->L, lua_gettop(core->L));
                                }
                                else
                                {

                                }
                            }
                            break;
                        case DT_DIR:
                        case DT_LNK:
                        default:
                            break;
                    }
                }
                closedir (dir);
            }

            nk_menu_end(core->ctx);
        }

        nk_menubar_end(core->ctx);

        if (SDL_TRUE == show_about)
        {
            if (nk_popup_begin(
                    core->ctx,
                    NK_POPUP_STATIC,
                    "About",
                    NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE,
                    nk_rect(10, 10, 420, 160)))
            {
                nk_layout_row_dynamic(core->ctx, 20, 1);
                nk_label(core->ctx, "CANopenTerm", NK_TEXT_CENTERED);
                nk_label(core->ctx, "Copyright (c) 2022, Michael Fitzmayer.", NK_TEXT_CENTERED);
                nk_label(core->ctx, "This project is licensed under the \"The MIT License\".", NK_TEXT_CENTERED);
                nk_label(core->ctx, "See https://github.com/mupfdev/CANopenTerm for details.", NK_TEXT_CENTERED);
                nk_popup_end(core->ctx);
            }
            else
            {
                show_about = SDL_FALSE;
            }
        }
    }
    nk_end(core->ctx);
}
