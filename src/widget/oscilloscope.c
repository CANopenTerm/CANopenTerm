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

static double oscilloscope_catmull_rom(double p0, double p1, double p2, double p3, double t)
{
    double t2 = t * t;
    double t3 = t2 * t;

    return 0.5 * (2.0 * p1 +
                  (-p0 + p2) * t +
                  (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 +
                  (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
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

        /* Scale the available width based on how many samples we actually have */
        uint32 actual_display_width = (samples_to_show * display_samples) / display_samples;
        if (buffer->size < display_samples)
        {
            actual_display_width = buffer->size;
        }

        /* Use Catmull-Rom spline interpolation for smooth curves */
        uint32 interpolation_steps = 32; /* Number of points between data points for very smooth curves */
        int first_point = 1;

        for (i = 0; i < samples_to_show - 1; i++)
        {
            /* Get four control points for the Catmull-Rom spline */
            /* p0: point before current */
            double p0 = (double)oscilloscope_buffer_get(buffer, buffer->size - samples_to_show + ((i > 0) ? i - 1 : i));
            /* p1: current point (start of curve segment) */
            double p1 = (double)oscilloscope_buffer_get(buffer, buffer->size - samples_to_show + i);
            /* p2: next point (end of curve segment) */
            double p2 = (double)oscilloscope_buffer_get(buffer, buffer->size - samples_to_show + i + 1);
            /* p3: point after next */
            double p3 = (double)oscilloscope_buffer_get(buffer, buffer->size - samples_to_show + ((i < samples_to_show - 2) ? i + 2 : i + 1));

            /* Clamp all control points to min/max range */
            if (p0 > buffer->max_value)
            {
                p0 = buffer->max_value;
            }
            if (p0 < buffer->min_value)
            {
                p0 = buffer->min_value;
            }
            if (p1 > buffer->max_value)
            {
                p1 = buffer->max_value;
            }
            if (p1 < buffer->min_value)
            {
                p1 = buffer->min_value;
            }
            if (p2 > buffer->max_value)
            {
                p2 = buffer->max_value;
            }
            if (p2 < buffer->min_value)
            {
                p2 = buffer->min_value;
            }
            if (p3 > buffer->max_value)
            {
                p3 = buffer->max_value;
            }
            if (p3 < buffer->min_value)
            {
                p3 = buffer->min_value;
            }

            /* Draw interpolated curve as pixels between p1 and p2 */
            for (uint32 step = 0; step <= interpolation_steps; step++)
            {
                double t = (double)step / interpolation_steps;
                double interpolated_value = oscilloscope_catmull_rom(p0, p1, p2, p3, t);

                /* Clamp interpolated value */
                if (interpolated_value > buffer->max_value)
                {
                    interpolated_value = buffer->max_value;
                }
                if (interpolated_value < buffer->min_value)
                {
                    interpolated_value = buffer->min_value;
                }

                plot_x = pos_x + 2 + ((i * interpolation_steps + step) * actual_display_width) / ((samples_to_show - 1) * interpolation_steps);
                plot_y = pos_y + height - 2 - ((interpolated_value - buffer->min_value) * (height - 4)) / value_range;

                /* Draw pixel at interpolated point */
                os_draw_pixel(renderer, plot_x, plot_y);
            }
        }
    }

    /* Display label, current value, and axis information */
    widget_print(pos_x + 2, pos_y + 2, DRAW_COLOR, 1u, "%s: %d/%d", label ? label : "OSC", current_value, buffer->max_value);

    /* Display Y-axis min/max values on the right side */
    widget_print(pos_x + width - 65, pos_y + 2, DRAW_COLOR, 1u, "Max: %05d", buffer->max_value);
    widget_print(pos_x + width - 65, pos_y + height - 11, DRAW_COLOR, 1u, "Min: %05d", buffer->min_value);
}
