/** @file led.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "os.h"
#include "palette.h"
#include "primitives.h"
#include "window.h"

void widget_led(uint32 pos_x, uint32 pos_y, uint32 size, bool state)
{
    os_renderer* renderer = window_get_renderer();
    uint32 draw_white_color = palette_get_color(DRAW_WHITE);
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint8 r, g, b;

    int center_x;
    int center_y;

    if (! renderer)
    {
        return;
    }

    if (true == state)
    {
        r = (draw_white_color & 0xff0000) >> 16;
        g = (draw_white_color & 0x00ff00) >> 8;
        b = (draw_white_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);
        draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, true);
    }

    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, pos_x + (size / 2), pos_y + (size / 2), size / 2, false);
}
