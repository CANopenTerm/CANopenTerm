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
    MIDNIGHT_BLUE = 0,
    DEEP_FOREST = 1,
    DARK_PLUM = 2,
    CHARCOAL_AMBER = 3,
    DARK_WINE = 4,
    DEERE_FIELD = 5,
    NEW_AMSTERDAM = 6,
    CRIMSON_HARVEST = 7,
    EMERALD_DRIVE = 8,
    IRON_HARVEST = 9

} pal_theme_t;

/* Color palette structure */
typedef struct pal_color_palette
{
    uint32 bg_color;
    uint32 widget_color;
    uint32 widget_color_highlight;
    uint32 draw_color;
    uint32 draw_color_alt;
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
    DRAW_COLOR_ALT = 4,
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
