/** @file palette.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PALETTE_H
#define PALETTE_H

#include "os.h"

/* Theme enumeration for widget_theme() API */
typedef enum pal_theme
{
    NEON_ECLIPSE = 0,
    OBSIDIAN_LUXE = 1,
    DEEP_OCEAN = 2,
    EMBER_CORE = 3,
    TOXIC_LAB = 4,

    /* Light themes */
    ARCTIC_DAY = 5,
    IVORY_LUXE = 6,
    SKYLINE = 7,
    SUNLIT_EMBER = 8,
    MINT_LAB = 9

} pal_theme_t;

/* Color palette structure */
typedef struct pal_color_palette
{
    uint32 bg_color;
    uint32 widget_color;
    uint32 widget_color_highlight;
    uint32 draw_color;
    uint32 draw_white;
    uint32 status_bar_low_1;
    uint32 status_bar_low_2;
    uint32 status_bar_mid_1;
    uint32 status_bar_mid_2;
    uint32 status_bar_high_1;
    uint32 status_bar_high_2;

} pal_color_palette_t;

/* Legacy color names for backward compatibility - these now refer to current theme */
typedef enum pal_color
{
    BG_COLOR = 0,
    WIDGET_COLOR = 1,
    WIDGET_COLOR_HIGHLIGHT = 2,
    DRAW_COLOR = 3,
    DRAW_WHITE = 4,
    STATUS_BAR_LOW_1 = 5,
    STATUS_BAR_LOW_2 = 6,
    STATUS_BAR_MID_1 = 7,
    STATUS_BAR_MID_2 = 8,
    STATUS_BAR_HIGH_1 = 9,
    STATUS_BAR_HIGH_2 = 10

} pal_color_t;

/* Theme management functions */
const pal_color_palette_t* palette_get_current_theme(void);
void palette_set_theme(pal_theme_t theme);
pal_theme_t palette_get_theme(void);
uint32 palette_get_color(pal_color_t color);

#endif /* PALETTE_H */
