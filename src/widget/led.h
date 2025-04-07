/** @file led.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef LED_H
#define LED_H

#include "os.h"

void widget_led(uint32 pos_x, uint32 pos_y, uint32 size, bool state);

#endif /* LED_H */
