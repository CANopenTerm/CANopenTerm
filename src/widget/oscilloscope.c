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

oscilloscope_ringbuffer_t* oscilloscope_buffer_create(uint32 min_value, uint32 max_value, uint32 size)
{
    oscilloscope_ringbuffer_t* buffer = (oscilloscope_ringbuffer_t*)os_calloc(1, sizeof(oscilloscope_ringbuffer_t));

    if (! buffer)
    {
        return NULL;
    }

    /* Validate size parameter */
    if (size == 0)
    {
        os_free(buffer);
        return NULL;
    }

    /* Allocate dynamic arrays */
    buffer->buffer = (uint32*)os_calloc(size, sizeof(uint32));
    buffer->timestamps = (uint64*)os_calloc(size, sizeof(uint64));

    if (! buffer->buffer || ! buffer->timestamps)
    {
        if (buffer->buffer)
        {
            os_free(buffer->buffer);
        }
        if (buffer->timestamps)
        {
            os_free(buffer->timestamps);
        }
        os_free(buffer);
        return NULL;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;
    buffer->capacity = size;
    buffer->min_value = min_value;
    buffer->max_value = max_value;

    return buffer;
}

void oscilloscope_buffer_destroy(oscilloscope_ringbuffer_t* buffer)
{
    if (buffer)
    {
        if (buffer->buffer)
        {
            os_free(buffer->buffer);
        }
        if (buffer->timestamps)
        {
            os_free(buffer->timestamps);
        }
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
    buffer->timestamps[buffer->head] = os_get_ticks();
    buffer->head = (buffer->head + 1) % buffer->capacity;

    if (buffer->size < buffer->capacity)
    {
        buffer->size++;
    }
    else
    {
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
    }
}

uint32 oscilloscope_buffer_get(oscilloscope_ringbuffer_t* buffer, uint32 index)
{
    if (! buffer || index >= buffer->size)
    {
        return 0;
    }

    uint32 actual_index = (buffer->tail + index) % buffer->capacity;
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

/* Catmull-Rom spline interpolation
 * Interpolates a point on a Catmull-Rom spline curve
 * p0, p1, p2, p3 are the control points
 * t is the interpolation parameter (0.0 to 1.0)
 * Returns the interpolated value
 */
static float oscilloscope_catmull_rom_interpolate(float p0, float p1, float p2, float p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    /* Catmull-Rom basis functions */
    float m0 = -0.5f * t3 + t2 - 0.5f * t;
    float m1 = 1.5f * t3 - 2.5f * t2 + 1.0f;
    float m2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
    float m3 = 0.5f * t3 - 0.5f * t2;

    return m0 * p0 + m1 * p1 + m2 * p2 + m3 * p3;
}

/* Get a clamped Y position for a given value
 * buffer: the oscilloscope buffer
 * value: the value to convert to Y position
 * pos_x, pos_y: widget position
 * height: widget height
 * Returns the Y coordinate as an integer
 */
static int oscilloscope_get_point_y(oscilloscope_ringbuffer_t* buffer, uint32 value, uint32 pos_y, uint32 height)
{
    uint32 value_range = buffer->max_value - buffer->min_value;
    if (value_range == 0)
    {
        value_range = 1;
    }

    /* Clamp value to buffer range */
    if (value > buffer->max_value)
    {
        value = buffer->max_value;
    }
    if (value < buffer->min_value)
    {
        value = buffer->min_value;
    }

    return pos_y + height - 2 - ((value - buffer->min_value) * (height - 4)) / value_range;
}

void widget_oscilloscope(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer, uint32 current_value, const char* label, uint64 time_window_ms)
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
    uint32 average_value;
    uint64 sum_value;
    int average_y_pos;

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
        uint64 current_time = os_get_ticks();
        uint64 time_window_ns;
        uint64 display_duration_ns;
        uint64 cutoff_time;
        uint32 display_width = width - 4;
        int plot_x_prev = -1;
        int plot_y_prev = -1;

        value_range = buffer->max_value - buffer->min_value;
        if (value_range == 0)
        {
            value_range = 1;
        }

        /* Calculate average value for all samples in buffer */
        sum_value = 0;
        for (i = 0; i < buffer->size; i++)
        {
            sum_value += oscilloscope_buffer_get(buffer, i);
        }
        average_value = (uint32)(sum_value / buffer->size);

        r = (draw_color & 0xff0000) >> 16;
        g = (draw_color & 0x00ff00) >> 8;
        b = (draw_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);

        if (time_window_ms > 0)
        {
            /* Time/division mode: display time_window_ms per grid division */
            /* 5 grid divisions, so total display time = 5 * time_window_ms */
            time_window_ns = time_window_ms * 1000000; /* Convert to nanoseconds */
            display_duration_ns = 5 * time_window_ns;  /* Total time to display */
            cutoff_time = (current_time > display_duration_ns) ? (current_time - display_duration_ns) : 0;

            /* Draw samples that fall within the time window using Catmull-Rom spline interpolation */
            int prev_point_index = -1;
            for (i = 0; i < buffer->size; i++)
            {
                uint32 actual_index = (buffer->tail + i) % buffer->capacity;
                uint64 sample_time = buffer->timestamps[actual_index];

                /* Only draw samples within the display window */
                if (sample_time >= cutoff_time)
                {
                    /* Calculate time offset from the start of display window */
                    uint64 time_offset = sample_time - cutoff_time;

                    /* Map time offset to X position */
                    plot_x = pos_x + 2 + (int)((time_offset * display_width) / display_duration_ns);

                    value = oscilloscope_buffer_get(buffer, i);
                    plot_y = oscilloscope_get_point_y(buffer, value, pos_y, height);

                    if (plot_x_prev >= 0 && prev_point_index >= 0 && i > 0)
                    {
                        /* Get control points for Catmull-Rom interpolation */
                        uint32 val_p0 = oscilloscope_buffer_get(buffer, i - 1);
                        uint32 val_p1 = val_p0;
                        uint32 val_p2 = value;
                        uint32 val_p3 = value;

                        if (i + 1 < buffer->size)
                        {
                            val_p3 = oscilloscope_buffer_get(buffer, i + 1);
                        }

                        /* Interpolate 7 points between the two data points */
                        for (int interp = 1; interp <= 7; interp++)
                        {
                            float t = (float)interp / 8.0f;
                            float interp_value = oscilloscope_catmull_rom_interpolate(
                                (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                            int interp_x = plot_x_prev + (plot_x - plot_x_prev) * interp / 8;
                            int interp_y = (int)(oscilloscope_get_point_y(buffer, (uint32)interp_value, pos_y, height));

                            os_draw_line(renderer, plot_x_prev, plot_y_prev, interp_x, interp_y);
                            plot_x_prev = interp_x;
                            plot_y_prev = interp_y;
                        }
                    }
                    else
                    {
                        if (plot_x_prev < 0)
                        {
                            plot_x_prev = plot_x;
                            plot_y_prev = plot_y;
                        }
                    }

                    prev_point_index = i;
                }
            }
        }
        else
        {
            /* Default mode (time_window_ms == 0): adaptive scaling to show all data with spline interpolation */
            display_samples = width - 4;
            samples_to_show = buffer->size;

            if (samples_to_show > display_samples)
            {
                samples_to_show = display_samples;
            }

            uint32 actual_display_width = (samples_to_show * display_samples) / display_samples;
            if (buffer->size < display_samples)
            {
                actual_display_width = buffer->size;
            }

            for (i = 0; i < samples_to_show; i++)
            {
                uint32 idx = buffer->size - samples_to_show + i;
                value = oscilloscope_buffer_get(buffer, idx);
                plot_x = pos_x + 2 + (i * actual_display_width) / samples_to_show;
                plot_y = oscilloscope_get_point_y(buffer, value, pos_y, height);

                if (i > 0 && i < samples_to_show - 1)
                {
                    /* Get control points for Catmull-Rom interpolation */
                    uint32 val_p0 = oscilloscope_buffer_get(buffer, idx - 1);
                    uint32 val_p1 = val_p0;
                    uint32 val_p2 = value;
                    uint32 val_p3 = oscilloscope_buffer_get(buffer, idx + 1);

                    int prev_x = pos_x + 2 + ((i - 1) * actual_display_width) / samples_to_show;
                    int prev_y = oscilloscope_get_point_y(buffer, val_p1, pos_y, height);

                    /* Interpolate 7 points between the two data points */
                    for (int interp = 1; interp <= 7; interp++)
                    {
                        float t = (float)interp / 8.0f;
                        float interp_value = oscilloscope_catmull_rom_interpolate(
                            (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                        int interp_x = prev_x + (plot_x - prev_x) * interp / 8;
                        int interp_y = (int)(oscilloscope_get_point_y(buffer, (uint32)interp_value, pos_y, height));

                        os_draw_line(renderer, prev_x, prev_y, interp_x, interp_y);
                        prev_x = interp_x;
                        prev_y = interp_y;
                    }
                }
                else if (i == 0 && samples_to_show > 1)
                {
                    /* First point - just store position */
                    prev_plot_x = plot_x;
                    prev_plot_y = plot_y;
                }
                else if (i > 0)
                {
                    /* Last point or single point - connect if needed */
                    if (i == 1 && samples_to_show == 2)
                    {
                        os_draw_line(renderer, prev_plot_x, prev_plot_y, plot_x, plot_y);
                    }
                }
            }
        }

        /* Draw average value line */
        average_y_pos = pos_y + height - 2 - ((average_value - buffer->min_value) * (height - 4)) / value_range;
        os_set_color(renderer, (draw_color & 0xff0000) >> 16, (draw_color & 0x00ff00) >> 8, (draw_color & 0x0000ff), 0x80);
        os_draw_line(renderer, pos_x + 2, average_y_pos, pos_x + width - 2, average_y_pos);
    }

    /* Display label, current value, and axis information */
    widget_print(pos_x + 2, pos_y + 2, DRAW_COLOR, 1u, "%s: %d/%d | Avg: %d", label ? label : "OSC", current_value, buffer->max_value, average_value);

    /* Display Y-axis min/max values on the right side */
    widget_print(pos_x + width - 65, pos_y + 2, DRAW_COLOR, 1u, "Max: %05d", buffer->max_value);
    widget_print(pos_x + width - 65, pos_y + height - 11, DRAW_COLOR, 1u, "Min: %05d", buffer->min_value);

    /* Display time-per-division scaling when in time/division mode */
    if (time_window_ms > 0)
    {
        widget_print(pos_x + 2, pos_y + height - 11, DRAW_COLOR, 1u, "%llu ms/div", time_window_ms);
    }
}

void widget_oscilloscope_2ch(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer1, uint32 current_value1, const char* label1, oscilloscope_ringbuffer_t* buffer2, uint32 current_value2, const char* label2, uint64 time_window_ms)
{
    os_renderer* renderer = window_get_renderer();
    os_rect box = {pos_x, pos_y, width, height};
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_color = palette_get_color(DRAW_COLOR);
    uint32 highlight_draw_color = palette_get_color(DRAW_COLOR_ALT);
    uint32 grid_color = palette_get_color(STATUS_BAR_LOW_1);
    uint8 r, g, b;
    uint32 i;
    uint32 display_samples;
    uint32 value;
    uint32 prev_value;
    uint32 y_pos;
    uint32 prev_y_pos;
    uint32 value_range1;
    uint32 value_range2;
    uint32 sample_width;
    int plot_x;
    int plot_y;
    int prev_plot_x;
    int prev_plot_y;
    uint32 samples_to_show;
    uint32 v_grid_spacing;
    uint32 v;
    uint64 sum_value;

    if (! renderer || ! buffer1 || ! buffer2)
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

    /* Draw waveforms for both buffers */
    if (buffer1->size > 0)
    {
        uint64 current_time = os_get_ticks();
        uint64 time_window_ns;
        uint64 display_duration_ns;
        uint64 cutoff_time;
        uint32 display_width = width - 4;
        int plot_x_prev_b1 = -1;
        int plot_y_prev_b1 = -1;
        int plot_x_prev_b2 = -1;
        int plot_y_prev_b2 = -1;

        value_range1 = buffer1->max_value - buffer1->min_value;
        if (value_range1 == 0)
        {
            value_range1 = 1;
        }

        value_range2 = buffer2->max_value - buffer2->min_value;
        if (value_range2 == 0)
        {
            value_range2 = 1;
        }

        /* Draw Buffer 1 (draw_color) */
        r = (draw_color & 0xff0000) >> 16;
        g = (draw_color & 0x00ff00) >> 8;
        b = (draw_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);

        if (time_window_ms > 0)
        {
            time_window_ns = time_window_ms * 1000000;
            display_duration_ns = 5 * time_window_ns;
            cutoff_time = (current_time > display_duration_ns) ? (current_time - display_duration_ns) : 0;

            int prev_point_index_b1 = -1;
            for (i = 0; i < buffer1->size; i++)
            {
                uint32 actual_index = (buffer1->tail + i) % buffer1->capacity;
                uint64 sample_time = buffer1->timestamps[actual_index];

                if (sample_time >= cutoff_time)
                {
                    uint64 time_offset = sample_time - cutoff_time;
                    plot_x = pos_x + 2 + (int)((time_offset * display_width) / display_duration_ns);

                    value = oscilloscope_buffer_get(buffer1, i);
                    plot_y = oscilloscope_get_point_y(buffer1, value, pos_y, height);

                    if (plot_x_prev_b1 >= 0 && prev_point_index_b1 >= 0 && i > 0)
                    {
                        uint32 val_p0 = oscilloscope_buffer_get(buffer1, i - 1);
                        uint32 val_p1 = val_p0;
                        uint32 val_p2 = value;
                        uint32 val_p3 = value;

                        if (i + 1 < buffer1->size)
                        {
                            val_p3 = oscilloscope_buffer_get(buffer1, i + 1);
                        }

                        for (int interp = 1; interp <= 7; interp++)
                        {
                            float t = (float)interp / 8.0f;
                            float interp_value = oscilloscope_catmull_rom_interpolate(
                                (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                            int interp_x = plot_x_prev_b1 + (plot_x - plot_x_prev_b1) * interp / 8;
                            int interp_y = (int)(oscilloscope_get_point_y(buffer1, (uint32)interp_value, pos_y, height));

                            os_draw_line(renderer, plot_x_prev_b1, plot_y_prev_b1, interp_x, interp_y);
                            plot_x_prev_b1 = interp_x;
                            plot_y_prev_b1 = interp_y;
                        }
                    }
                    else
                    {
                        if (plot_x_prev_b1 < 0)
                        {
                            plot_x_prev_b1 = plot_x;
                            plot_y_prev_b1 = plot_y;
                        }
                    }

                    prev_point_index_b1 = i;
                }
            }
        }
        else
        {
            display_samples = width - 4;
            samples_to_show = buffer1->size;

            if (samples_to_show > display_samples)
            {
                samples_to_show = display_samples;
            }

            uint32 actual_display_width = (samples_to_show * display_samples) / display_samples;
            if (buffer1->size < display_samples)
            {
                actual_display_width = buffer1->size;
            }

            for (i = 0; i < samples_to_show; i++)
            {
                uint32 idx = buffer1->size - samples_to_show + i;
                value = oscilloscope_buffer_get(buffer1, idx);
                plot_x = pos_x + 2 + (i * actual_display_width) / samples_to_show;
                plot_y = oscilloscope_get_point_y(buffer1, value, pos_y, height);

                if (i > 0 && i < samples_to_show - 1)
                {
                    uint32 val_p0 = oscilloscope_buffer_get(buffer1, idx - 1);
                    uint32 val_p1 = val_p0;
                    uint32 val_p2 = value;
                    uint32 val_p3 = oscilloscope_buffer_get(buffer1, idx + 1);

                    int prev_x = pos_x + 2 + ((i - 1) * actual_display_width) / samples_to_show;
                    int prev_y = oscilloscope_get_point_y(buffer1, val_p1, pos_y, height);

                    for (int interp = 1; interp <= 7; interp++)
                    {
                        float t = (float)interp / 8.0f;
                        float interp_value = oscilloscope_catmull_rom_interpolate(
                            (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                        int interp_x = prev_x + (plot_x - prev_x) * interp / 8;
                        int interp_y = (int)(oscilloscope_get_point_y(buffer1, (uint32)interp_value, pos_y, height));

                        os_draw_line(renderer, prev_x, prev_y, interp_x, interp_y);
                        prev_x = interp_x;
                        prev_y = interp_y;
                    }
                }
                else if (i == 0 && samples_to_show > 1)
                {
                    plot_x_prev_b1 = plot_x;
                    plot_y_prev_b1 = plot_y;
                }
                else if (i > 0)
                {
                    if (i == 1 && samples_to_show == 2)
                    {
                        os_draw_line(renderer, plot_x_prev_b1, plot_y_prev_b1, plot_x, plot_y);
                    }
                }
            }
        }

        /* Draw Buffer 2 (highlight_draw_color) */
        r = (highlight_draw_color & 0xff0000) >> 16;
        g = (highlight_draw_color & 0x00ff00) >> 8;
        b = (highlight_draw_color & 0x0000ff);

        os_set_color(renderer, r, g, b, 0xff);

        if (time_window_ms > 0)
        {
            time_window_ns = time_window_ms * 1000000;
            display_duration_ns = 5 * time_window_ns;
            cutoff_time = (current_time > display_duration_ns) ? (current_time - display_duration_ns) : 0;

            int prev_point_index_b2 = -1;
            for (i = 0; i < buffer2->size; i++)
            {
                uint32 actual_index = (buffer2->tail + i) % buffer2->capacity;
                uint64 sample_time = buffer2->timestamps[actual_index];

                if (sample_time >= cutoff_time)
                {
                    uint64 time_offset = sample_time - cutoff_time;
                    plot_x = pos_x + 2 + (int)((time_offset * display_width) / display_duration_ns);

                    value = oscilloscope_buffer_get(buffer2, i);
                    plot_y = oscilloscope_get_point_y(buffer2, value, pos_y, height);

                    if (plot_x_prev_b2 >= 0 && prev_point_index_b2 >= 0 && i > 0)
                    {
                        uint32 val_p0 = oscilloscope_buffer_get(buffer2, i - 1);
                        uint32 val_p1 = val_p0;
                        uint32 val_p2 = value;
                        uint32 val_p3 = value;

                        if (i + 1 < buffer2->size)
                        {
                            val_p3 = oscilloscope_buffer_get(buffer2, i + 1);
                        }

                        for (int interp = 1; interp <= 7; interp++)
                        {
                            float t = (float)interp / 8.0f;
                            float interp_value = oscilloscope_catmull_rom_interpolate(
                                (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                            int interp_x = plot_x_prev_b2 + (plot_x - plot_x_prev_b2) * interp / 8;
                            int interp_y = (int)(oscilloscope_get_point_y(buffer2, (uint32)interp_value, pos_y, height));

                            os_draw_line(renderer, plot_x_prev_b2, plot_y_prev_b2, interp_x, interp_y);
                            plot_x_prev_b2 = interp_x;
                            plot_y_prev_b2 = interp_y;
                        }
                    }
                    else
                    {
                        if (plot_x_prev_b2 < 0)
                        {
                            plot_x_prev_b2 = plot_x;
                            plot_y_prev_b2 = plot_y;
                        }
                    }

                    prev_point_index_b2 = i;
                }
            }
        }
        else
        {
            display_samples = width - 4;
            samples_to_show = buffer2->size;

            if (samples_to_show > display_samples)
            {
                samples_to_show = display_samples;
            }

            uint32 actual_display_width = (samples_to_show * display_samples) / display_samples;
            if (buffer2->size < display_samples)
            {
                actual_display_width = buffer2->size;
            }

            for (i = 0; i < samples_to_show; i++)
            {
                uint32 idx = buffer2->size - samples_to_show + i;
                value = oscilloscope_buffer_get(buffer2, idx);
                plot_x = pos_x + 2 + (i * actual_display_width) / samples_to_show;
                plot_y = oscilloscope_get_point_y(buffer2, value, pos_y, height);

                if (i > 0 && i < samples_to_show - 1)
                {
                    uint32 val_p0 = oscilloscope_buffer_get(buffer2, idx - 1);
                    uint32 val_p1 = val_p0;
                    uint32 val_p2 = value;
                    uint32 val_p3 = oscilloscope_buffer_get(buffer2, idx + 1);

                    int prev_x = pos_x + 2 + ((i - 1) * actual_display_width) / samples_to_show;
                    int prev_y = oscilloscope_get_point_y(buffer2, val_p1, pos_y, height);

                    for (int interp = 1; interp <= 7; interp++)
                    {
                        float t = (float)interp / 8.0f;
                        float interp_value = oscilloscope_catmull_rom_interpolate(
                            (float)val_p0, (float)val_p1, (float)val_p2, (float)val_p3, t);

                        int interp_x = prev_x + (plot_x - prev_x) * interp / 8;
                        int interp_y = (int)(oscilloscope_get_point_y(buffer2, (uint32)interp_value, pos_y, height));

                        os_draw_line(renderer, prev_x, prev_y, interp_x, interp_y);
                        prev_x = interp_x;
                        prev_y = interp_y;
                    }
                }
                else if (i == 0 && samples_to_show > 1)
                {
                    plot_x_prev_b2 = plot_x;
                    plot_y_prev_b2 = plot_y;
                }
                else if (i > 0)
                {
                    if (i == 1 && samples_to_show == 2)
                    {
                        os_draw_line(renderer, plot_x_prev_b2, plot_y_prev_b2, plot_x, plot_y);
                    }
                }
            }
        }
    }

    /* Display labels and values (no average bar) */
    widget_print(pos_x + 2, pos_y + 2, DRAW_COLOR, 1u, "%s: %d/%d", label1 ? label1 : "OSC1", current_value1, buffer1->max_value);
    widget_print(pos_x + 2, pos_y + 12, WIDGET_COLOR_HIGHLIGHT, 1u, "%s: %d/%d", label2 ? label2 : "OSC2", current_value2, buffer2->max_value);

    /* Display Y-axis min/max values on the right side for buffer1 */
    widget_print(pos_x + width - 65, pos_y + 2, DRAW_COLOR, 1u, "Max1: %05d", buffer1->max_value);
    widget_print(pos_x + width - 65, pos_y + height - 11, DRAW_COLOR, 1u, "Min1: %05d", buffer1->min_value);

    /* Display Y-axis min/max values for buffer2 */
    widget_print(pos_x + width - 140, pos_y + 2, WIDGET_COLOR_HIGHLIGHT, 1u, "Max2: %05d", buffer2->max_value);
    widget_print(pos_x + width - 140, pos_y + height - 11, WIDGET_COLOR_HIGHLIGHT, 1u, "Min2: %05d", buffer2->min_value);

    /* Display time-per-division scaling when in time/division mode */
    if (time_window_ms > 0)
    {
        widget_print(pos_x + 2, pos_y + height - 22, DRAW_COLOR, 1u, "%llu ms/div", time_window_ms);
    }
}
