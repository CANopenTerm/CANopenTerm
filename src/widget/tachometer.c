/** @file tachometer.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

/* TODO: Use OS-abstraction layer. */

#include "tachometer.h"
#include "os.h"
#include "palette.h"
#include "window.h"
#include <SDL3/SDL.h>

void widget_tachometer(uint8 pos_x, uint8 pos_y, uint8 w, uint8 h, const uint32 max, uint32 rpm)
{
    SDL_FRect rect = {pos_x, pos_y, w, h};
    SDL_Renderer* renderer = window_get_renderer();

    uint8 r, g, b;

    float angle;
    float radians;
    int center_x;
    int center_y;
    int needle_length;
    int needle_x;
    int needle_y;

    if (! renderer)
    {
        return;
    }

    r = (WIDGET_COLOR & 0xff0000) >> 16;
    g = (WIDGET_COLOR & 0x00ff00) >> 8;
    b = (WIDGET_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderFillRect(renderer, &rect);

    r = (WIDGET_COLOR_HIGHLIGHT & 0xff0000) >> 16;
    g = (WIDGET_COLOR_HIGHLIGHT & 0x00ff00) >> 8;
    b = (WIDGET_COLOR_HIGHLIGHT & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderRect(renderer, &rect);

    angle = 180.0f - ((float)rpm / max * 180.0f);
    radians = angle * (3.14159f / 180.0f);

    center_x = pos_x + w / 2;
    center_y = pos_y + h / 2;
    needle_length = (w < h ? w : h) / 2 - 10;

    needle_x = center_x + (int)(needle_length * SDL_cos(radians));
    needle_y = center_y - (int)(needle_length * SDL_sin(radians));

    r = (DRAW_COLOR & 0xff0000) >> 16;
    g = (DRAW_COLOR & 0x00ff00) >> 8;
    b = (DRAW_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderLine(renderer, center_x, center_y, needle_x, needle_y);
}
