/** @file window_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "os.h"
#include "palette.h"
#include <SDL3/SDL.h>

void window_clear(void)
{
    extern core_t* core;

    uint8 r = (BG_COLOR & 0xff0000) >> 16;
    uint8 g = (BG_COLOR & 0x00ff00) >> 8;
    uint8 b = (BG_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(core->renderer, r, g, b, 0xff);
    SDL_RenderClear(core->renderer);
}

os_renderer* window_get_renderer(void)
{
    extern core_t* core;
    return core->renderer;
}

void window_hide(void)
{
    extern core_t* core;
    if (core->window)
    {
        SDL_HideWindow(core->window);
        core->is_window_shown = false;
    }
}

void window_fullscreen(bool state)
{
    extern core_t* core;
    if (core->window)
    {
        core->is_window_fullscreen = state;
        SDL_SetWindowFullscreen(core->window, state);
    }
}

bool window_init(core_t* core)
{
    if (! core)
    {
        return false;
    }

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

    SDL_SetWindowPosition(core->window, (mode->w - mode->h / 2) - 64, 64);
    return true;
}

void window_show(void)
{
    extern core_t* core;
    if (core->window)
    {
        SDL_ShowWindow(core->window);
        core->is_window_shown = true;
    }
}

status_t window_update(void)
{
    extern core_t* core;
    if (core->renderer)
    {
        SDL_RenderPresent(core->renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_KEY_DOWN:
                {
                    if (event.key.repeat) /* No key repeat. */
                    {
                        break;
                    }
                    switch (event.key.key)
                    {
                        case SDLK_ESCAPE:
                        {
                            if (true == core->is_window_fullscreen)
                            {
                                window_fullscreen(false);
                            }
                            break;
                        }
                        case SDLK_F11:
                        {
                            if (true == core->is_window_fullscreen)
                            {
                                window_fullscreen(false);
                            }
                            else
                            {
                                window_fullscreen(true);
                            }
                            break;
                        }
                    }
                    break;
                }
                case SDL_EVENT_QUIT:
                    return CORE_QUIT;
            }
        }
    }

    return ALL_OK;
}
