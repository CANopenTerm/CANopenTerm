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
    BG_COLOR = 0x121212,
    WIDGET_COLOR = 0x1e1e1e,
    WIDGET_COLOR_HIGHLIGHT = 0xbb86fc,
    DRAW_COLOR = 0x03daC6,
    STATUS_BAR_LOW_1 = 0x00c853,
    STATUS_BAR_LOW_2 = 0x64dd17,
    STATUS_BAR_MID_1 = 0xffd600,
    STATUS_BAR_MID_2 = 0xff9100,
    STATUS_BAR_HIGH_1 = 0xff1744,
    STATUS_BAR_HIGH_2 = 0xd500f9

} pal_color_t;

#endif /* PALETTE_H */
