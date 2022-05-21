/** @file gui.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef GUI_H
#define GUI_H

#include "SDL.h"
#include "core.h"

status_t gui_init(core_t* core);
status_t gui_update(core_t* core);
void     gui_deinit(core_t* core);
SDL_bool is_gui_active(core_t* core);

#endif /* GUI_H */
