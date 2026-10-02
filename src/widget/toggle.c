/** @file toggle.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "toggle.h"
#include "os.h"
#include "palette.h"
#include "primitives.h"
#include "window.h"

#define MAX_TOGGLE_WIDGETS 32

/* Global toggle widget registry */
static toggle_widget_t g_toggle_widgets[MAX_TOGGLE_WIDGETS] = {0};
static bool g_toggle_initialized[MAX_TOGGLE_WIDGETS] = {false};

uint32 widget_toggle_register(uint32 pos_x, uint32 pos_y, uint32 size, bool initial_state)
{
    for (uint32 i = 0; i < MAX_TOGGLE_WIDGETS; i++)
    {
        if (! g_toggle_initialized[i])
        {
            g_toggle_widgets[i].pos_x = pos_x;
            g_toggle_widgets[i].pos_y = pos_y;
            g_toggle_widgets[i].size = size;
            g_toggle_widgets[i].state = initial_state;
            g_toggle_widgets[i].is_hovered = false;
            g_toggle_widgets[i].callback = NULL;
            g_toggle_initialized[i] = true;
            return i;
        }
    }
    return (uint32)-1;
}

void widget_toggle_unregister(uint32 toggle_id)
{
    if (toggle_id < MAX_TOGGLE_WIDGETS)
    {
        g_toggle_initialized[toggle_id] = false;
        g_toggle_widgets[toggle_id].callback = NULL;
    }
}

void widget_toggle(uint32 pos_x, uint32 pos_y, uint32 size, bool state)
{
    os_renderer* renderer = window_get_renderer();
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_white_color = palette_get_color(DRAW_COLOR_ALT);
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint8 r, g, b;
    os_rect toggle_rect;
    int center_y;
    int toggle_height;
    int toggle_width;
    int knob_radius;
    int knob_x;
    int padding;
    int i;

    if (! renderer)
    {
        return;
    }

    /* Calculate toggle dimensions */
    padding = 2;
    toggle_height = (size * 3) / 5; /* Height is 60% of size */
    toggle_width = size * 2;        /* Width is 2x the size */
    center_y = pos_y + (size / 2);
    knob_radius = (toggle_height - (padding * 2)) / 2;

    /* Draw frame rect */
    toggle_rect.x = pos_x;
    toggle_rect.y = center_y - (toggle_height / 2);
    toggle_rect.w = toggle_width;
    toggle_rect.h = toggle_height;

    /* Draw background of toggle (darker when off, lighter when on) */
    if (true == state)
    {
        r = (draw_white_color & 0xff0000) >> 16;
        g = (draw_white_color & 0x00ff00) >> 8;
        b = (draw_white_color & 0x0000ff);
        r = (r > 50) ? r - 50 : 0;
        g = (g > 50) ? g - 50 : 0;
        b = (b > 50) ? b - 50 : 0;
    }
    else
    {
        r = (widget_color & 0xff0000) >> 16;
        g = (widget_color & 0x00ff00) >> 8;
        b = (widget_color & 0x0000ff);
        r = (r > 20) ? r - 20 : 0;
        g = (g > 20) ? g - 20 : 0;
        b = (b > 20) ? b - 20 : 0;
    }

    os_set_color(renderer, r, g, b, 0xff);
    /* Draw filled rectangle by drawing horizontal lines */
    for (i = 0; i < toggle_height; i++)
    {
        os_draw_line(renderer, pos_x, center_y - (toggle_height / 2) + i,
                     pos_x + toggle_width, center_y - (toggle_height / 2) + i);
    }

    /* Draw toggle border */
    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);
    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &toggle_rect);

    /* Draw knob */
    if (true == state)
    {
        knob_x = pos_x + toggle_width - knob_radius - padding;
    }
    else
    {
        knob_x = pos_x + knob_radius + padding;
    }

    r = (draw_white_color & 0xff0000) >> 16;
    g = (draw_white_color & 0x00ff00) >> 8;
    b = (draw_white_color & 0x0000ff);
    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, knob_x, center_y, knob_radius, true);

    /* Draw knob highlight */
    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);
    os_set_color(renderer, r, g, b, 0xff);
    draw_circle(renderer, knob_x, center_y, knob_radius, false);
}

void widget_toggle_set_callback(uint32 toggle_id, toggle_callback_t callback)
{
    if (toggle_id < MAX_TOGGLE_WIDGETS && g_toggle_initialized[toggle_id])
    {
        g_toggle_widgets[toggle_id].callback = callback;
    }
}

bool widget_toggle_get_state(uint32 toggle_id)
{
    if (toggle_id < MAX_TOGGLE_WIDGETS && g_toggle_initialized[toggle_id])
    {
        return g_toggle_widgets[toggle_id].state;
    }
    return false;
}

void widget_toggle_set_state(uint32 toggle_id, bool state)
{
    if (toggle_id < MAX_TOGGLE_WIDGETS && g_toggle_initialized[toggle_id])
    {
        g_toggle_widgets[toggle_id].state = state;
    }
}

static bool widget_toggle_point_inside(uint32 toggle_id, uint32 mouse_x, uint32 mouse_y)
{
    if (toggle_id >= MAX_TOGGLE_WIDGETS || ! g_toggle_initialized[toggle_id])
    {
        return false;
    }

    toggle_widget_t* toggle = &g_toggle_widgets[toggle_id];
    uint32 toggle_height = (toggle->size * 3) / 5;
    uint32 toggle_width = toggle->size * 2;
    uint32 toggle_top = toggle->pos_y + (toggle->size / 2) - (toggle_height / 2);

    return (mouse_x >= toggle->pos_x &&
            mouse_x <= toggle->pos_x + toggle_width &&
            mouse_y >= toggle_top &&
            mouse_y <= toggle_top + toggle_height);
}

bool widget_toggle_check_click(uint32 mouse_x, uint32 mouse_y)
{
    /* Check all registered toggles in reverse order (last drawn should be checked first) */
    for (int i = MAX_TOGGLE_WIDGETS - 1; i >= 0; i--)
    {
        if (g_toggle_initialized[i] && widget_toggle_point_inside(i, mouse_x, mouse_y))
        {
            /* Toggle the state */
            g_toggle_widgets[i].state = ! g_toggle_widgets[i].state;

            /* Call callback if registered */
            if (g_toggle_widgets[i].callback)
            {
                g_toggle_widgets[i].callback(i, g_toggle_widgets[i].state);
            }

            return true;
        }
    }
    return false;
}

void widget_toggle_update(void)
{
    /* Reserved for future use - handle animations, hover states, etc. */
}

void widget_toggle_set_position(uint32 toggle_id, uint32 pos_x, uint32 pos_y)
{
    if (toggle_id < MAX_TOGGLE_WIDGETS && g_toggle_initialized[toggle_id])
    {
        g_toggle_widgets[toggle_id].pos_x = pos_x;
        g_toggle_widgets[toggle_id].pos_y = pos_y;
    }
}
