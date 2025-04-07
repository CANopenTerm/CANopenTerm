/** @file led.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

/* TODO: Use OS-abstraction layer. */

#include <SDL3/SDL.h>

#include "os.h"
#include "palette.h"
#include "primitives.h"
#include "window.h"

void widget_led(uint32 pos_x, uint32 pos_y, uint32 size, bool state)
{
    SDL_Renderer* renderer = window_get_renderer();
    pal_color_t color = WIDGET_COLOR;
    uint8 r, g, b;

    int center_x;
    int center_y;

    if (! renderer)
    {
        return;
    }

    if (true == state)
    {
        r = (DRAW_WHITE & 0xff0000) >> 16;
        g = (DRAW_WHITE & 0x00ff00) >> 8;
        b = (DRAW_WHITE & 0x0000ff);

        SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
        draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, true);
    }

    r = (WIDGET_COLOR_HIGHLIGHT & 0xff0000) >> 16;
    g = (WIDGET_COLOR_HIGHLIGHT & 0x00ff00) >> 8;
    b = (WIDGET_COLOR_HIGHLIGHT & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, false);
}
