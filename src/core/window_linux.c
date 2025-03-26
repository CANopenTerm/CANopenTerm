/** @file window_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"

bool window_init(core_t* core)
{
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(1);
    if (! mode)
    {
        return false;
    }

    if (! SDL_CreateWindowAndRenderer(
            "CANopenTerm", mode->h / 2, mode->h / 2,
            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY,
            &core->window, &core->renderer))
    {
        return false;
    }

    return true;
}

void window_deinit(core_t* core)
{
    SDL_DestroyWindow(core->window);
    SDL_DestroyRenderer(core->renderer);
    core->window = NULL;
    core->renderer = NULL;
    core->is_window_shown = false;
}

void window_hide(core_t* core)
{
    if (core->window)
    {
        SDL_HideWindow(core->window);
        core->is_window_shown = false;
    }
}

void window_show(core_t* core)
{
    if (core->window)
    {
        SDL_ShowWindow(core->window);
        core->is_window_shown = true;
    }
}

void window_update(core_t* core)
{
    if (core->renderer)
    {
        SDL_RenderPresent(core->renderer);
    }
}
