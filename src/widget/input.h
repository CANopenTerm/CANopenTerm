/** @file input.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef INPUT_H
#define INPUT_H

#include "os.h"

#define MAX_INPUT_WIDGETS 32
#define MAX_INPUT_BUFFER_SIZE 256

typedef void (*input_callback_t)(uint32 input_id, const char* text);

typedef struct input_widget
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 width;
    uint32 height;
    char buffer[MAX_INPUT_BUFFER_SIZE];
    uint32 buffer_length;
    uint32 cursor_pos;
    bool is_active;
    bool is_hovered;
    input_callback_t callback;

} input_widget_t;

uint32 widget_input_register(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const char* initial_text);
void widget_input_unregister(uint32 input_id);
void widget_input(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const char* text, bool is_active);
void widget_input_set_callback(uint32 input_id, input_callback_t callback);
const char* widget_input_get_text(uint32 input_id);
bool widget_input_is_active(uint32 input_id);
void widget_input_set_text(uint32 input_id, const char* text);
bool widget_input_check_click(uint32 mouse_x, uint32 mouse_y);
void widget_input_handle_text(const char* text);
void widget_input_handle_backspace(void);
void widget_input_handle_return(void);
void widget_input_update(void);
void widget_input_set_position(uint32 input_id, uint32 pos_x, uint32 pos_y);
void widget_input_deactivate_all(void);

#endif /* INPUT_H */
