/** @file core.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdio.h>
#include "core.h"
#include "command.h"
#include "can.h"
#include "gui.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

status_t core_init(core_t **core)
{
    *core = (core_t*)calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        return COT_ERROR;
    }

    puts("CANopenTerm\r\n");

    // Initialise SDL.
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(0, "Unable to initialise video sub-system: %s", SDL_GetError());
        return COT_ERROR;
    }

    // Initialise Lua.
    (*core)->L = luaL_newstate();
    luaL_openlibs((*core)->L);
    lua_register_nmt_command((*core));

    // Initialise CAN.
    can_init((*core));

    (*core)->is_running = SDL_TRUE;
    return COT_OK;
}

status_t core_update(core_t *core)
{
    char command[64] = { 0 };

    if (NULL == core)
    {
        return COT_OK;
    }

    if (is_gui_active(core))
    {
        if (COT_QUIT == gui_update(core))
        {
            gui_deinit(core);
        }
        else
        {
            return COT_OK;
        }
    }

    printf(": ");
    gets(command);
    parse_command(command, core);

    return COT_OK;
}

void core_deinit(core_t *core)
{
    if (NULL == core)
    {
        return;
    }

    if (is_gui_active(core))
    {
        gui_deinit(core);
    }

    can_deinit(core);
    lua_close(core->L);
    free(core);

    SDL_Quit();
}
