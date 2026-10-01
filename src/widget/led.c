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
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_white_color = palette_get_color(DRAW_COLOR_ALT);
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint8 r, g, b;
    os_rect frame_rect;
    int center_x;
    int center_y;
    int led_radius;
    int padding;

    if (! renderer)
    {
        return;
    }

    /* Calculate frame dimensions with padding */
    padding = 4;
    led_radius = (size - (padding * 2)) / 2;
    center_x = pos_x + (size / 2);
    center_y = pos_y + (size / 2);

    /* Draw frame rect for reference */
    frame_rect.x = pos_x;
    frame_rect.y = pos_y;
    frame_rect.w = size;
    frame_rect.h = size;

    /* Draw LED circle - fill with state color if on */
    if (true == state)
    {
        r = (draw_white_color & 0xff0000) >> 16;
        g = (draw_white_color & 0x00ff00) >> 8;
        b = (draw_white_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);
        draw_circle(renderer, center_x, center_y, led_radius, true);
    }
    else
    {
        /* Draw darker circle when off */
        char temp_r = (widget_color & 0xff0000) >> 16;
        char temp_g = (widget_color & 0x00ff00) >> 8;
        char temp_b = (widget_color & 0x0000ff);
        temp_r = (temp_r > 20) ? temp_r - 20 : 0;
        temp_g = (temp_g > 20) ? temp_g - 20 : 0;
        temp_b = (temp_b > 20) ? temp_b - 20 : 0;

        os_set_color(renderer, temp_r, temp_g, temp_b, 0xff);
        draw_circle(renderer, center_x, center_y, led_radius, true);
    }

    /* Draw LED circle highlight/border */
    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, center_x, center_y, led_radius, false);

    /* Draw frame border - simple single line like oscilloscope */
    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &frame_rect);
}
