/** @file oscilloscope.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "oscilloscope.h"
#include "ascii.h"
#include "os.h"
#include "palette.h"
#include "window.h"

oscilloscope_ringbuffer_t* oscilloscope_buffer_create(uint32 min_value, uint32 max_value)
{
    oscilloscope_ringbuffer_t* buffer = (oscilloscope_ringbuffer_t*)os_calloc(1, sizeof(oscilloscope_ringbuffer_t));

    if (! buffer)
    {
        return NULL;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;
    buffer->min_value = min_value;
    buffer->max_value = max_value;

    return buffer;
}

void oscilloscope_buffer_destroy(oscilloscope_ringbuffer_t* buffer)
{
    if (buffer)
    {
        os_free(buffer);
    }
}

void oscilloscope_buffer_push(oscilloscope_ringbuffer_t* buffer, uint32 value)
{
    if (! buffer)
    {
        return;
    }

    buffer->buffer[buffer->head] = value;
    buffer->head = (buffer->head + 1) % OSCILLOSCOPE_BUFFER_SIZE;

    if (buffer->size < OSCILLOSCOPE_BUFFER_SIZE)
    {
        buffer->size++;
    }
    else
    {
        buffer->tail = (buffer->tail + 1) % OSCILLOSCOPE_BUFFER_SIZE;
    }
}

uint32 oscilloscope_buffer_get(oscilloscope_ringbuffer_t* buffer, uint32 index)
{
    if (! buffer || index >= buffer->size)
    {
        return 0;
    }

    uint32 actual_index = (buffer->tail + index) % OSCILLOSCOPE_BUFFER_SIZE;
    return buffer->buffer[actual_index];
}

uint32 oscilloscope_buffer_get_size(oscilloscope_ringbuffer_t* buffer)
{
    if (! buffer)
    {
        return 0;
    }

    return buffer->size;
}

void widget_oscilloscope(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer, uint32 current_value, const char* label)
{
    os_renderer* renderer = window_get_renderer();
    os_rect box = {pos_x, pos_y, width, height};
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_color = palette_get_color(DRAW_COLOR);
    uint32 grid_color = palette_get_color(STATUS_BAR_LOW_1);
    uint8 r, g, b;
    uint32 i;
    uint32 display_samples;
    uint32 value;
    uint32 prev_value;
    uint32 y_pos;
    uint32 prev_y_pos;
    uint32 value_range;
    uint32 sample_width;
    int plot_x;
    int plot_y;
    int prev_plot_x;
    int prev_plot_y;
    uint32 samples_to_show;
    uint32 v_grid_spacing;
    uint32 v;

    if (! renderer || ! buffer)
    {
        return;
    }

    /* Draw background */
    r = (widget_color & 0xff0000) >> 16;
    g = (widget_color & 0x00ff00) >> 8;
    b = (widget_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    os_draw_fill_rect(renderer, &box);

    /* Draw border */
    r = (highlight_color & 0xff0000) >> 16;
    g = (highlight_color & 0x00ff00) >> 8;
    b = (highlight_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &box);

    /* Draw grid lines */
    r = (grid_color & 0xff0000) >> 16;
    g = (grid_color & 0x00ff00) >> 8;
    b = (grid_color & 0x0000ff);

    os_set_color(renderer, r, g, b, 0x80);

    /* Horizontal grid lines - quarters */
    os_draw_line(renderer, pos_x + 1, pos_y + height / 4, pos_x + width - 2, pos_y + height / 4);
    os_draw_line(renderer, pos_x + 1, pos_y + height / 2, pos_x + width - 2, pos_y + height / 2);
    os_draw_line(renderer, pos_x + 1, pos_y + 3 * height / 4, pos_x + width - 2, pos_y + 3 * height / 4);

    /* Vertical grid lines */
    v_grid_spacing = (width - 4) / 5;
    for (v = 1; v < 5; v++)
    {
        os_draw_line(renderer, pos_x + 2 + (v * v_grid_spacing), pos_y + 1, pos_x + 2 + (v * v_grid_spacing), pos_y + height - 2);
    }

    /* Draw waveform */
    if (buffer->size > 0)
    {
        display_samples = width - 4;
        samples_to_show = buffer->size;

        if (samples_to_show > display_samples)
        {
            samples_to_show = display_samples;
        }

        value_range = buffer->max_value - buffer->min_value;
        if (value_range == 0)
        {
            value_range = 1;
        }

        r = (draw_color & 0xff0000) >> 16;
        g = (draw_color & 0x00ff00) >> 8;
        b = (draw_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);

        for (i = 0; i < samples_to_show; i++)
        {
            value = oscilloscope_buffer_get(buffer, buffer->size - samples_to_show + i);

            if (value > buffer->max_value)
            {
                value = buffer->max_value;
            }
            if (value < buffer->min_value)
            {
                value = buffer->min_value;
            }

            plot_x = pos_x + 2 + (i * display_samples) / samples_to_show;
            plot_y = pos_y + height - 2 - ((value - buffer->min_value) * (height - 4)) / value_range;

            if (i > 0)
            {
                os_draw_line(renderer, prev_plot_x, prev_plot_y, plot_x, plot_y);
            }

            prev_plot_x = plot_x;
            prev_plot_y = plot_y;
        }
    }

    /* Display label, current value, and axis information */
    widget_print(pos_x + 2, pos_y + 2, DRAW_COLOR, 1u, "%s: %d/%d", label ? label : "OSC", current_value, buffer->max_value);

    /* Display Y-axis min/max values on the right side */
    widget_print(pos_x + width - 65, pos_y + 2, DRAW_COLOR, 1u, "Max: %05d", buffer->max_value);
    widget_print(pos_x + width - 65, pos_y + height - 11, DRAW_COLOR, 1u, "Min: %05d", buffer->min_value);
}
