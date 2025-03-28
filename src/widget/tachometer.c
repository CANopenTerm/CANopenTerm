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

static void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius, bool fill)
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y)
    {
        if (fill)
        {
            for (int i = (cx - x); i <= (cx + x); i++)
            {
                SDL_RenderPoint(renderer, i, cy + y);
                SDL_RenderPoint(renderer, i, cy - y);
            }
            for (int i = (cx - y); i <= (cx + y); i++)
            {
                SDL_RenderPoint(renderer, i, cy + x);
                SDL_RenderPoint(renderer, i, cy - x);
            }
        }
        else
        {
            SDL_RenderPoint(renderer, cx + x, cy + y);
            SDL_RenderPoint(renderer, cx - x, cy + y);
            SDL_RenderPoint(renderer, cx + x, cy - y);
            SDL_RenderPoint(renderer, cx - x, cy - y);
            SDL_RenderPoint(renderer, cx + y, cy + x);
            SDL_RenderPoint(renderer, cx - y, cy + x);
            SDL_RenderPoint(renderer, cx + y, cy - x);
            SDL_RenderPoint(renderer, cx - y, cy - x);
        }

        if (d < 0)
        {
            d += 4 * x + 6;
        }
        else
        {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void widget_tachometer(uint32 pos_x, uint32 pos_y, uint32 size, const uint32 max, uint32 value)
{
    SDL_Renderer* renderer = window_get_renderer();
    uint8 r, g, b;

    float angle;
    float radians;
    int center_x;
    int center_y;
    int needle_length;
    int needle_x;
    int needle_y;
    int indicator_length = size / 20;

    if (! renderer)
    {
        return;
    }

    r = (WIDGET_COLOR & 0xff0000) >> 16;
    g = (WIDGET_COLOR & 0x00ff00) >> 8;
    b = (WIDGET_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, true);

    r = (WIDGET_COLOR_HIGHLIGHT & 0xff0000) >> 16;
    g = (WIDGET_COLOR_HIGHLIGHT & 0x00ff00) >> 8;
    b = (WIDGET_COLOR_HIGHLIGHT & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, false);
    SDL_RenderLine(renderer, pos_x, pos_y + (size / 2), pos_x + indicator_length, pos_y + (size / 2));
    SDL_RenderLine(renderer, pos_x + (size - indicator_length), pos_y + (size / 2), pos_x + size, pos_y + (size / 2));

    angle = 180.0f - ((float)value / max * 180.0f);
    radians = angle * (3.14159f / 180.0f);

    center_x = pos_x + size / 2;
    center_y = pos_y + size / 2;
    needle_length = size / 2;

    needle_x = center_x + (int)(needle_length * SDL_cos(radians));
    needle_y = center_y - (int)(needle_length * SDL_sin(radians));

    r = (DRAW_COLOR & 0xff0000) >> 16;
    g = (DRAW_COLOR & 0x00ff00) >> 8;
    b = (DRAW_COLOR & 0x0000ff);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
    SDL_RenderLine(renderer, center_x, center_y, needle_x, needle_y);
}
