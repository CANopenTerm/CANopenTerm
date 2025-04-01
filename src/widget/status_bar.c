/** @file status_bar.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

/* TODO: Use OS-abstraction layer. */

#include <SDL3/SDL.h>

#include "ascii.h"
#include "os.h"
#include "palette.h"
#include "window.h"

void widget_status_bar(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const uint32 max, uint32 value)
{
    SDL_Renderer* renderer = window_get_renderer();
    SDL_FRect box = {pos_x, pos_y, width, height};
    pal_color_t colors[] = {STATUS_BAR_LOW_1, STATUS_BAR_LOW_2, STATUS_BAR_MID_1, STATUS_BAR_MID_2, STATUS_BAR_HIGH_1, STATUS_BAR_HIGH_2};
    pal_color_t color;
    uint32 i;
    uint32 num_bars = width - 2;
    uint32 filled_bars;
    uint32 color_index;
    uint8 r, g, b;

    if (! renderer || max == 0)
    {
        return;
    }

    r = (WIDGET_COLOR & 0xff0000) >> 16;
    g = (WIDGET_COLOR & 0x00ff00) >> 8;
    b = (WIDGET_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderFillRect(renderer, &box);

    r = (WIDGET_COLOR_HIGHLIGHT & 0xff0000) >> 16;
    g = (WIDGET_COLOR_HIGHLIGHT & 0x00ff00) >> 8;
    b = (WIDGET_COLOR_HIGHLIGHT & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderRect(renderer, &box);

    num_bars = width - 2;
    filled_bars = (value * num_bars) / max;

    color_index = (filled_bars * 6) / num_bars;
    if (color_index > 5)
    {
        color_index = 5;
    }

    color = colors[color_index];

    r = (color & 0xff0000) >> 16;
    g = (color & 0x00ff00) >> 8;
    b = (color & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    for (i = 0; i < filled_bars; i++)
    {
        SDL_RenderLine(renderer, pos_x + 1 + i, pos_y + 1, pos_x + 1 + i, pos_y + height - 2);
    }

    widget_print(pos_x + 2, pos_y + 2, DRAW_WHITE, "%d/%d", value, max);
}
