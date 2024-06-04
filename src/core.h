/** @file core.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CORE_H
#define CORE_H

#include "SDL.h"
#include "lua.h"

typedef enum status
{
    COT_OK = 0,
    COT_WARNING,
    COT_ERROR,
    COT_QUIT

} status_t;

typedef struct core
{
    SDL_Window        *window;
    SDL_Renderer      *renderer;
    SDL_Thread        *can_monitor_th;
    lua_State         *L;
    struct nk_context *ctx;
    Uint8              baud_rate;
    Uint32             can_status;
    Uint8              node_id;
    SDL_bool           is_can_initialised;
    SDL_bool           is_running;
    SDL_bool           is_gui_active;
    SDL_bool           is_script_running;
#ifdef USE_LIBSOCKETCAN
    char can_interface[32];
#endif
} core_t;

status_t core_init(core_t **core);
status_t core_update(core_t *core);
void     core_deinit(core_t *core);

#endif /* CORE_H */
