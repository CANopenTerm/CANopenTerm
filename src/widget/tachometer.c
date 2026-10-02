/** @file tachometer.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "tachometer.h"
#include "ascii.h"
#include "os.h"
#include "palette.h"
#include "primitives.h"
#include "window.h"

void widget_tachometer(uint32 pos_x, uint32 pos_y, uint32 size, const uint32 max, uint32 value)
{
    os_renderer* renderer = window_get_renderer();
    uint8 r, g, b;

    char buffer[10] = {0};
    float angle;
    float radians;
    int center_x;
    int center_y;
    int needle_length;
    int needle_x;
    int needle_y;
    uint8 scale = 2u;

    size_t len;
    int text_width;

    /* Variables for decorative ticks */
    int i;
    int tick_count = 12;
    float tick_angle;
    float tick_radians;
    int tick_outer_x;
    int tick_outer_y;
    int tick_inner_x;
    int tick_inner_y;
    int tick_radius_outer;
    int tick_radius_inner;

    if (size <= 100)
    {
        scale = 1u;
    }

    if (! renderer)
    {
        return;
    }

    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_color = palette_get_color(DRAW_COLOR);

    center_x = pos_x + size / 2;
    center_y = pos_y + size / 2;

    /* Create frame rect for simple border */
    os_rect frame_rect = {pos_x, pos_y, size, size};

    /* Gauge circle with small gap from frame */
    int gauge_radius = (size / 2) - 4;

    /* Draw background circle */
    r = (widget_color & 0xff0000) >> 16;
    g = (widget_color & 0x00ff00) >> 8;
    b = (widget_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, center_x, center_y, gauge_radius, true);

    /* Draw border circle */
    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, center_x, center_y, gauge_radius, false);

    /* Draw frame border */
    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &frame_rect);

    /* Draw decorative tick marks around the gauge */
    tick_radius_outer = gauge_radius - 1;
    tick_radius_inner = gauge_radius - (size / 8);

    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);

    for (i = 0; i < tick_count; i++)
    {
        tick_angle = 180.0f - ((float)i / (tick_count - 1) * 180.0f);
        tick_radians = tick_angle * (3.14159f / 180.0f);

        tick_outer_x = center_x + (int)(tick_radius_outer * os_cos(tick_radians));
        tick_outer_y = center_y - (int)(tick_radius_outer * os_sin(tick_radians));
        tick_inner_x = center_x + (int)(tick_radius_inner * os_cos(tick_radians));
        tick_inner_y = center_y - (int)(tick_radius_inner * os_sin(tick_radians));

        os_draw_line(renderer, tick_outer_x, tick_outer_y, tick_inner_x, tick_inner_y);
    }

    /* Draw center circle for needle pivot */
    int pivot_radius = 3 + (size / 100);
    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, center_x, center_y, pivot_radius, true);

    /* Draw indicator marks at edges using same tick length */
    int indicator_outer_x_left = center_x + (int)(tick_radius_outer * os_cos(3.14159f));
    int indicator_inner_x_left = center_x + (int)(tick_radius_inner * os_cos(3.14159f));
    int indicator_outer_x_right = center_x + (int)(tick_radius_outer * os_cos(0.0f));
    int indicator_inner_x_right = center_x + (int)(tick_radius_inner * os_cos(0.0f));

    os_draw_line(renderer, indicator_outer_x_left, pos_y + (size / 2), indicator_inner_x_left, pos_y + (size / 2));
    os_draw_line(renderer, indicator_inner_x_right, pos_y + (size / 2), indicator_outer_x_right, pos_y + (size / 2));

    /* Calculate and draw needle */
    angle = 180.0f - ((float)value / max * 180.0f);
    radians = angle * (3.14159f / 180.0f);

    /* Make needle extend almost to the edge of the gauge for better visibility */
    needle_length = gauge_radius - 2;

    needle_x = center_x + (int)(needle_length * os_cos(radians));
    needle_y = center_y - (int)(needle_length * os_sin(radians));

    r = (draw_color & 0xff0000) >> 16;
    g = (draw_color & 0x00ff00) >> 8;
    b = (draw_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    os_draw_line(renderer, center_x, center_y, needle_x, needle_y);

    /* Display value */
    os_snprintf(buffer, 9, "%Xh", value);
    len = os_strlen(buffer);
    text_width = (CHAR_WIDTH + CHAR_SPACING) * scale * len - (CHAR_SPACING);

    widget_print(
        center_x - (text_width / 2),
        center_y + (CHAR_HEIGHT * 2),
        DRAW_COLOR, scale, "%s", buffer);
}
