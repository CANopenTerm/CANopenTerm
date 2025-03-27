/** @file tachometer.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TACHOMETER_H
#define TACHOMETER_H

#include "os.h"

void widget_tachometer(uint8 pos_x, uint8 pos_y, uint8 size, const uint32 max, uint32 rpm);

#endif /* TACHOMETER_H */
