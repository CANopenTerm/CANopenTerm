/** @file toggle.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TOGGLE_H
#define TOGGLE_H

#include "os.h"

/* Forward declaration for callback function pointer */
typedef void (*toggle_callback_t)(uint32 toggle_id, bool state);

/* Toggle widget structure */
typedef struct toggle_widget
{
    uint32 pos_x;
    uint32 pos_y;
    uint32 size;
    bool state;
    bool is_hovered;
    toggle_callback_t callback;
} toggle_widget_t;

/* Register a toggle widget and return its ID */
uint32 widget_toggle_register(uint32 pos_x, uint32 pos_y, uint32 size, bool initial_state);

/* Unregister a toggle widget by ID */
void widget_toggle_unregister(uint32 toggle_id);

/* Draw a toggle widget */
void widget_toggle(uint32 pos_x, uint32 pos_y, uint32 size, bool state);

/* Set the callback for a toggle widget */
void widget_toggle_set_callback(uint32 toggle_id, toggle_callback_t callback);

/* Get the state of a toggle widget */
bool widget_toggle_get_state(uint32 toggle_id);

/* Set the state of a toggle widget */
void widget_toggle_set_state(uint32 toggle_id, bool state);

/* Check if a position is over a toggle widget and toggle if clicked */
bool widget_toggle_check_click(uint32 mouse_x, uint32 mouse_y);

/* Update all toggle widgets (handle internal state) */
void widget_toggle_update(void);

/* Update the position of a toggle widget */
void widget_toggle_set_position(uint32 toggle_id, uint32 pos_x, uint32 pos_y);

#endif /* TOGGLE_H */
