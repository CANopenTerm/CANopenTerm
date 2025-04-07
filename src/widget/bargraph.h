/** @file bargraph.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef BARGRAPH_H
#define BARGRAPH_H

#include "os.h"

void widget_bargraph(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, const uint32 max, uint32 value);

#endif /* BARGRAPH_H */
