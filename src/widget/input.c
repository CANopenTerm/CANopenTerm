/** @file input.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "input.h"
#include "ascii.h"
#include "core.h"
#include "os.h"
#include "palette.h"
#include "primitives.h"
#include "window.h"

static input_widget_t input_widgets[MAX_INPUT_WIDGETS] = {0};
static bool input_initialized[MAX_INPUT_WIDGETS] = {false};
static uint32 active_input_id = (uint32)-1;

uint32 widget_input_register(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const char* initial_text)
{
    uint32 i;

    for (i = 0; i < MAX_INPUT_WIDGETS; i++)
    {
        if (! input_initialized[i])
        {
            input_widgets[i].pos_x = pos_x;
            input_widgets[i].pos_y = pos_y;
            input_widgets[i].width = width;
            input_widgets[i].height = height;
            input_widgets[i].is_active = false;
            input_widgets[i].is_hovered = false;
            input_widgets[i].callback = NULL;
            input_widgets[i].buffer_length = 0;
            input_widgets[i].cursor_pos = 0;
            os_memset(input_widgets[i].buffer, 0, MAX_INPUT_BUFFER_SIZE);

            if (initial_text)
            {
                os_strlcpy(input_widgets[i].buffer, initial_text, MAX_INPUT_BUFFER_SIZE);
                input_widgets[i].buffer_length = os_strlen(initial_text);
                input_widgets[i].cursor_pos = input_widgets[i].buffer_length;
            }

            input_initialized[i] = true;
            return i;
        }
    }
    return (uint32)-1;
}

void widget_input_unregister(uint32 input_id)
{
    if (input_id < MAX_INPUT_WIDGETS)
    {
        input_initialized[input_id] = false;
        input_widgets[input_id].callback = NULL;

        if (active_input_id == input_id)
        {
            active_input_id = (uint32)-1;
        }
    }
}

void widget_input(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const char* text, bool is_active)
{
    os_renderer* renderer = window_get_renderer();
    uint32 highlight_color = palette_get_color(WIDGET_COLOR_HIGHLIGHT);
    uint32 draw_color = palette_get_color(DRAW_COLOR);
    uint32 draw_color_alt = palette_get_color(DRAW_COLOR_ALT);
    uint32 widget_color = palette_get_color(WIDGET_COLOR);
    uint8 r, g, b;
    os_rect input_rect;
    int padding;
    int i;
    uint8 scale;

    if (! renderer)
    {
        return;
    }

    padding = 2;

    /* Draw background */
    input_rect.x = pos_x;
    input_rect.y = pos_y;
    input_rect.w = width;
    input_rect.h = height;

    if (is_active)
    {
        r = (draw_color_alt & 0xff0000) >> 16;
        g = (draw_color_alt & 0x00ff00) >> 8;
        b = (draw_color_alt & 0x0000ff);
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

    /* Draw filled rectangle */
    for (i = 0; i < (int)height; i++)
    {
        os_draw_line(renderer, pos_x, pos_y + i, pos_x + width, pos_y + i);
    }

    /* Draw border */
    if (is_active)
    {
        r = (highlight_color & 0xff0000) >> 16;
        g = (highlight_color & 0x00ff00) >> 8;
        b = (highlight_color & 0x0000ff);
    }
    else
    {
        r = (draw_color_alt & 0xff0000) >> 16;
        g = (draw_color_alt & 0x00ff00) >> 8;
        b = (draw_color_alt & 0x0000ff);
    }
    os_set_color(renderer, r, g, b, 0xff);
    os_draw_rect(renderer, &input_rect);

    /* Draw text if provided */
    if (text || is_active)
    {
        int text_height;
        int vertical_offset;
        int left_margin;
        int available_width;
        int char_width;
        uint32 max_displayable_chars;

        /* Calculate best scale factor based on widget height */
        /* Available height = height - 2*padding */
        /* Per line height at scale N = (CHAR_HEIGHT + LINE_SPACING) * scale */
        uint32 available_height = (height > 2 * (uint32)padding) ? height - 2 * padding : 1;
        scale = (uint8)((available_height) / (CHAR_HEIGHT + LINE_SPACING));
        if (scale > 1)
        {
            scale--;
        }
        if (scale < 1)
        {
            scale = 1;
        }

        /* Calculate vertical centering */
        text_height = (CHAR_HEIGHT * scale);
        vertical_offset = ((int)height - text_height) / 2;
        if (vertical_offset < padding)
        {
            vertical_offset = padding;
        }

        left_margin = CHAR_SPACING * 2 * scale;

        /* Calculate maximum displayable characters based on available width */
        available_width = (int)width - left_margin - (int)(CHAR_SPACING * scale);
        if (available_width < 0)
        {
            available_width = 0;
        }
        char_width = (CHAR_WIDTH + CHAR_SPACING) * scale;
        max_displayable_chars = (char_width > 0) ? available_width / char_width : 0;

        if (text && os_strlen(text) > 0)
        {
            if (is_active)
            {
                /* Display text with cursor at cursor_pos */
                char display_text[MAX_INPUT_BUFFER_SIZE + 1] = {0};
                size_t text_len = os_strlen(text);

                /* Limit display to max displayable characters */
                size_t display_len = (text_len > max_displayable_chars) ? max_displayable_chars : text_len;
                if (display_len > 0)
                {
                    os_memcpy(display_text, text, display_len);
                }
                display_text[display_len] = '\0';

                widget_print((int)(pos_x + left_margin), (int)(pos_y + vertical_offset), BG_COLOR, scale, "%s", display_text);

                /* Draw cursor as underscore at cursor position */
                if (display_len < max_displayable_chars)
                {
                    int cursor_x = (int)(pos_x + left_margin + (display_len * char_width));
                    widget_print(cursor_x, (int)(pos_y + vertical_offset), BG_COLOR, scale, "_");
                }
            }
            else
            {
                /* Display text without cursor when not active */
                char display_text[MAX_INPUT_BUFFER_SIZE + 1] = {0};
                size_t text_len = os_strlen(text);

                /* Limit display to max displayable characters */
                size_t display_len = (text_len > max_displayable_chars) ? max_displayable_chars : text_len;
                if (display_len > 0)
                {
                    os_memcpy(display_text, text, display_len);
                }
                display_text[display_len] = '\0';

                widget_print((int)(pos_x + left_margin), (int)(pos_y + vertical_offset), DRAW_COLOR, scale, "%s", display_text);
            }
        }
        else if (is_active)
        {
            /* Draw cursor even when text is empty */
            widget_print((int)(pos_x + left_margin), (int)(pos_y + vertical_offset), BG_COLOR, scale, "_");
        }
    }
}

void widget_input_set_callback(uint32 input_id, input_callback_t callback)
{
    if (input_id < MAX_INPUT_WIDGETS && input_initialized[input_id])
    {
        input_widgets[input_id].callback = callback;
    }
}

const char* widget_input_get_text(uint32 input_id)
{
    if (input_id < MAX_INPUT_WIDGETS && input_initialized[input_id])
    {
        return input_widgets[input_id].buffer;
    }
    return "";
}

bool widget_input_is_active(uint32 input_id)
{
    if (input_id < MAX_INPUT_WIDGETS && input_initialized[input_id])
    {
        return input_widgets[input_id].is_active;
    }
    return false;
}

void widget_input_set_text(uint32 input_id, const char* text)
{
    if (input_id < MAX_INPUT_WIDGETS && input_initialized[input_id])
    {
        input_widgets[input_id].buffer_length = 0;
        input_widgets[input_id].cursor_pos = 0;
        os_memset(input_widgets[input_id].buffer, 0, MAX_INPUT_BUFFER_SIZE);

        if (text)
        {
            os_strlcpy(input_widgets[input_id].buffer, text, MAX_INPUT_BUFFER_SIZE);
            input_widgets[input_id].buffer_length = os_strlen(text);
            input_widgets[input_id].cursor_pos = input_widgets[input_id].buffer_length;
        }
    }
}

static bool widget_input_point_inside(uint32 input_id, uint32 mouse_x, uint32 mouse_y)
{
    input_widget_t* input;

    if (input_id >= MAX_INPUT_WIDGETS || ! input_initialized[input_id])
    {
        return false;
    }

    input = &input_widgets[input_id];

    return (mouse_x >= input->pos_x &&
            mouse_x <= input->pos_x + input->width &&
            mouse_y >= input->pos_y &&
            mouse_y <= input->pos_y + input->height);
}

static uint32 widget_input_get_max_displayable_chars(uint32 input_id)
{
    input_widget_t* input;
    int padding;
    int left_margin;
    int available_width;
    int char_width;
    uint32 max_displayable_chars;
    uint32 available_height;
    uint8 scale;

    if (input_id >= MAX_INPUT_WIDGETS || ! input_initialized[input_id])
    {
        return 0;
    }

    input = &input_widgets[input_id];

    /* Calculate scale factor based on widget height */
    padding = 2;
    available_height = (input->height > 2 * (uint32)padding) ? input->height - 2 * padding : 1;
    scale = (uint8)((available_height) / (CHAR_HEIGHT + LINE_SPACING));
    if (scale > 1)
    {
        scale--;
    }
    if (scale < 1)
    {
        scale = 1;
    }

    left_margin = CHAR_SPACING * 2 * scale;
    available_width = (int)input->width - left_margin - (int)(CHAR_SPACING * scale);
    if (available_width < 0)
    {
        available_width = 0;
    }
    char_width = (CHAR_WIDTH + CHAR_SPACING) * scale;
    max_displayable_chars = (char_width > 0) ? available_width / char_width : 0;

    return max_displayable_chars;
}

bool widget_input_check_click(uint32 mouse_x, uint32 mouse_y)
{
    int i;

    /* Deactivate previously active input */
    if (active_input_id != (uint32)-1)
    {
        if (active_input_id < MAX_INPUT_WIDGETS && input_initialized[active_input_id])
        {
            input_widgets[active_input_id].is_active = false;
            if (core && core->window)
            {
                os_stop_text_input(core->window);
            }
        }
    }

    /* Check all registered inputs in reverse order */
    for (i = MAX_INPUT_WIDGETS - 1; i >= 0; i--)
    {
        if (input_initialized[i] && widget_input_point_inside(i, mouse_x, mouse_y))
        {
            /* Activate this input */
            input_widgets[i].is_active = true;
            active_input_id = i;

            /* Clear input field when activated */
            os_memset(input_widgets[i].buffer, 0, MAX_INPUT_BUFFER_SIZE);
            input_widgets[i].buffer_length = 0;
            input_widgets[i].cursor_pos = 0;

            if (core && core->window)
            {
                os_start_text_input(core->window);
            }
            return true;
        }
    }

    active_input_id = (uint32)-1;
    return false;
}

void widget_input_handle_text(const char* text)
{
    input_widget_t* input;
    uint32 max_displayable_chars;

    if (active_input_id == (uint32)-1 || text == NULL)
    {
        return;
    }

    if (active_input_id >= MAX_INPUT_WIDGETS || ! input_initialized[active_input_id])
    {
        return;
    }

    input = &input_widgets[active_input_id];
    max_displayable_chars = widget_input_get_max_displayable_chars(active_input_id);

    /* Add character if there's space in buffer and won't exceed displayable length */
    if (input->buffer_length < MAX_INPUT_BUFFER_SIZE - 1)
    {
        size_t text_len = os_strlen(text);
        if (text_len > 0 && input->buffer_length + text_len < MAX_INPUT_BUFFER_SIZE &&
            input->buffer_length < max_displayable_chars)
        {
            os_strlcat(input->buffer, text, MAX_INPUT_BUFFER_SIZE);
            input->buffer_length += text_len;
            input->cursor_pos = input->buffer_length;
        }
    }
}

void widget_input_handle_backspace(void)
{
    input_widget_t* input;

    if (active_input_id == (uint32)-1)
    {
        return;
    }

    if (active_input_id >= MAX_INPUT_WIDGETS || ! input_initialized[active_input_id])
    {
        return;
    }

    input = &input_widgets[active_input_id];

    /* Remove last character and update cursor position */
    if (input->buffer_length > 0)
    {
        input->buffer[input->buffer_length - 1] = '\0';
        input->buffer_length--;
        if (input->cursor_pos > 0)
        {
            input->cursor_pos--;
        }
    }
}

void widget_input_handle_return(void)
{
    input_widget_t* input;

    if (active_input_id == (uint32)-1)
    {
        return;
    }

    if (active_input_id >= MAX_INPUT_WIDGETS || ! input_initialized[active_input_id])
    {
        return;
    }

    input = &input_widgets[active_input_id];

    /* Call callback if registered */
    if (input->callback)
    {
        input->callback(active_input_id, input->buffer);
    }

    /* Deactivate input */
    input->is_active = false;
    active_input_id = (uint32)-1;
}

void widget_input_update(void)
{
    /* Reserved for future use; handle animations, cursor blinking, etc. */
}

void widget_input_set_position(uint32 input_id, uint32 pos_x, uint32 pos_y)
{
    if (input_id < MAX_INPUT_WIDGETS && input_initialized[input_id])
    {
        input_widgets[input_id].pos_x = pos_x;
        input_widgets[input_id].pos_y = pos_y;
    }
}

void widget_input_deactivate_all(void)
{
    uint32 i;

    for (i = 0; i < MAX_INPUT_WIDGETS; i++)
    {
        if (input_initialized[i])
        {
            input_widgets[i].is_active = false;
        }
    }
    active_input_id = (uint32)-1;
    if (core && core->window)
    {
        os_stop_text_input(core->window);
    }
}
