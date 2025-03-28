/** @file palette.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PALETTE_H
#define PALETTE_H

typedef enum pal_color
{
    BG_COLOR = 0x2d3142,
    WIDGET_COLOR = 0x086375,
    WIDGET_COLOR_HIGHLIGHT = 0x1dd3b0,
    DRAW_COLOR = 0xaffc41,
    STATUS_BAR_LOW_1 = 0x43aa8b,
    STATUS_BAR_LOW_2 = 0x90be6d,
    STATUS_BAR_MID_1 = 0xf9c74f,
    STATUS_BAR_MID_2 = 0xf8961e,
    STATUS_BAR_HIGH_1 = 0xf3722c,
    STATUS_BAR_HIGH_2 = 0xf94144

} pal_color_t;

#endif /* PALETTE_H */
