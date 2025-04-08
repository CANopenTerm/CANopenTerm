/** @file ascii.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef ASCII_H
#define ASCII_H

#include <stdarg.h>

#include "os.h"
#include "palette.h"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7

void widget_print(int x, int y, pal_color_t color, uint8 scale, const char* fmt, ...);

#endif /* ASCII_H */
