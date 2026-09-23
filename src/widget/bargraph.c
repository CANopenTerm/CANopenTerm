/** @file bargraph.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ascii.h"
#include "os.h"
#include "palette.h"
#include "window.h"

void widget_bargraph(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const uint32 max, uint32 value)
{
    os_renderer* renderer = window_get_renderer();
    os_rect box = {pos_x, pos_y, width, height};
    pal_color_t color_indices[] = {STATUS_BAR_LOW_1, STATUS_BAR_LOW_2, STATUS_BAR_MID_1, STATUS_BAR_MID_2, STATUS_BAR_HIGH_1, STATUS_BAR_HIGH_2};
    uint32 color;
    uint32 i;
    uint32 num_bars = width - 2;
    uint32 filled_bars;
    uint32 color_index;
    uint8 r, g, b;
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_white = palette_get_color(DRAW_WHITE);

    if (! renderer || max == 0)
    {
        return;
    }

    r = (widget_color & 0xff0000) >> 16;
    g = (widget_color & 0x00ff00) >> 8;
    b = (widget_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    os_draw_fill_rect(renderer, &box);

    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &box);

    num_bars = width - 2;
    filled_bars = (value * num_bars) / max;

    color_index = (filled_bars * 6) / num_bars;
    if (color_index > 5)
    {
        color_index = 5;
    }

    color = palette_get_color(color_indices[color_index]);

    r = (color & 0xff0000) >> 16;
    g = (color & 0x00ff00) >> 8;
    b = (color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    for (i = 0; i < filled_bars; i++)
    {
        os_draw_line(renderer, pos_x + 1 + i, pos_y + 1, pos_x + 1 + i, pos_y + height - 2);
    }

    widget_print(pos_x + 2, pos_y + 2, draw_white, 1u, "%d/%d", value, max);
}
