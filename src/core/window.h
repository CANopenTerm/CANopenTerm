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

bool window_init(core_t* core);
bool window_deinit(core_t* core);
void window_hide(core_t* core);
void window_show(core_t* core);
void window_update(core_t* core);
