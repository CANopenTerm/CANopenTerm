/** @file window.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "os.h"

void window_clear(void);
bool window_deinit(core_t* core);
os_renderer* window_get_renderer(void);
void window_hide(void);
bool window_init(core_t* core);
void window_show(void);
status_t window_update(void);
