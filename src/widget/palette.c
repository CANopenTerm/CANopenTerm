/** @file palette.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "palette.h"
#include "os.h"

/* Theme palettes */
static const pal_color_palette_t themes[] = {
    /* NEON_ECLIPSE */
    {
        .bg_color = 0x121212,
        .widget_color = 0x1e1e1e,
        .widget_color_highlight = 0xbb86fc,
        .draw_color = 0x03daC6,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x00c853,
        .status_bar_low_2 = 0x64dd17,
        .status_bar_mid_1 = 0xffd600,
        .status_bar_mid_2 = 0xff9100,
        .status_bar_high_1 = 0xff1744,
        .status_bar_high_2 = 0xd500f9},
    /* OBSIDIAN_LUXE */
    {
        .bg_color = 0x111111,
        .widget_color = 0x242424,
        .widget_color_highlight = 0xd4af37,
        .draw_color = 0xffffff,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x2e7d32,
        .status_bar_low_2 = 0x66bb6a,
        .status_bar_mid_1 = 0xf9a825,
        .status_bar_mid_2 = 0xff8f00,
        .status_bar_high_1 = 0xc62828,
        .status_bar_high_2 = 0x8e24aa},
    /* DEEP_OCEAN */
    {
        .bg_color = 0x08141c,
        .widget_color = 0x102733,
        .widget_color_highlight = 0x26a6d1,
        .draw_color = 0x00e5ff,
        .draw_white = 0xe8faff,
        .status_bar_low_1 = 0x00c9a7,
        .status_bar_low_2 = 0x69f0ae,
        .status_bar_mid_1 = 0xffd740,
        .status_bar_mid_2 = 0xffa726,
        .status_bar_high_1 = 0xff5252,
        .status_bar_high_2 = 0xce93d8},
    /* EMBER_CORE */
    {
        .bg_color = 0x15100e,
        .widget_color = 0x271a16,
        .widget_color_highlight = 0xff7043,
        .draw_color = 0xffd180,
        .draw_white = 0xfff8f0,
        .status_bar_low_1 = 0x43a047,
        .status_bar_low_2 = 0x8bc34a,
        .status_bar_mid_1 = 0xffd600,
        .status_bar_mid_2 = 0xff6d00,
        .status_bar_high_1 = 0xf4511e,
        .status_bar_high_2 = 0xd50000},
    /* TOXIC_LAB */
    {
        .bg_color = 0x0b1009,
        .widget_color = 0x16200f,
        .widget_color_highlight = 0x76ff03,
        .draw_color = 0xb2ff59,
        .draw_white = 0xf1ffe8,
        .status_bar_low_1 = 0x00c853,
        .status_bar_low_2 = 0x64dd17,
        .status_bar_mid_1 = 0xffd600,
        .status_bar_mid_2 = 0xff9100,
        .status_bar_high_1 = 0xff1744,
        .status_bar_high_2 = 0xd500f9},
    /* ARCTIC_DAY */
    {
        .bg_color = 0xf4f8fb,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x1976d2,
        .draw_color = 0x006064,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x2e7d32,
        .status_bar_low_2 = 0x66bb6a,
        .status_bar_mid_1 = 0xf9a825,
        .status_bar_mid_2 = 0xef6c00,
        .status_bar_high_1 = 0xd32f2f,
        .status_bar_high_2 = 0x7b1fa2},
    /* IVORY_LUXE */
    {
        .bg_color = 0xf8f5ee,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0xb8860b,
        .draw_color = 0x3e3a35,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x388e3c,
        .status_bar_low_2 = 0x689f38,
        .status_bar_mid_1 = 0xf9a825,
        .status_bar_mid_2 = 0xef6c00,
        .status_bar_high_1 = 0xc62828,
        .status_bar_high_2 = 0x8e24aa},
    /* SKYLINE */
    {
        .bg_color = 0xeaf4fb,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x0288d1,
        .draw_color = 0x01579b,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x00897b,
        .status_bar_low_2 = 0x43a047,
        .status_bar_mid_1 = 0xfbc02d,
        .status_bar_mid_2 = 0xfb8c00,
        .status_bar_high_1 = 0xe53935,
        .status_bar_high_2 = 0x8e24aa},
    /* SUNLIT_EMBER */
    {
        .bg_color = 0xfff7ed,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0xe65100,
        .draw_color = 0xbf360c,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x558b2f,
        .status_bar_low_2 = 0x7cb342,
        .status_bar_mid_1 = 0xf9a825,
        .status_bar_mid_2 = 0xef6c00,
        .status_bar_high_1 = 0xd84315,
        .status_bar_high_2 = 0xad1457},
    /* MINT_LAB */
    {
        .bg_color = 0xf2f8f1,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x558b2f,
        .draw_color = 0x33691e,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x00897b,
        .status_bar_low_2 = 0x43a047,
        .status_bar_mid_1 = 0xf9a825,
        .status_bar_mid_2 = 0xfb8c00,
        .status_bar_high_1 = 0xe53935,
        .status_bar_high_2 = 0x8e24aa}};

/* Current theme (default to NEON_ECLIPSE) */
static pal_theme_t current_theme = NEON_ECLIPSE;

/**
 * @brief Get the current theme palette
 * @return Pointer to the current theme's color palette
 */
const pal_color_palette_t* palette_get_current_theme(void)
{
    return &themes[current_theme];
}

/**
 * @brief Set the current theme
 * @param theme The theme to set
 */
void palette_set_theme(pal_theme_t theme)
{
    if (theme >= NEON_ECLIPSE && theme <= MINT_LAB)
    {
        current_theme = theme;
    }
}

/**
 * @brief Get the current theme identifier
 * @return The current theme ID
 */
pal_theme_t palette_get_theme(void)
{
    return current_theme;
}

/**
 * @brief Get a specific color from the current theme
 * @param color The color index
 * @return The 24-bit color value
 */
uint32 palette_get_color(pal_color_t color)
{
    const pal_color_palette_t* theme = &themes[current_theme];

    switch (color)
    {
        case BG_COLOR:
            return theme->bg_color;
        case WIDGET_COLOR:
            return theme->widget_color;
        case WIDGET_COLOR_HIGHLIGHT:
            return theme->widget_color_highlight;
        case DRAW_COLOR:
            return theme->draw_color;
        case DRAW_WHITE:
            return theme->draw_white;
        case STATUS_BAR_LOW_1:
            return theme->status_bar_low_1;
        case STATUS_BAR_LOW_2:
            return theme->status_bar_low_2;
        case STATUS_BAR_MID_1:
            return theme->status_bar_mid_1;
        case STATUS_BAR_MID_2:
            return theme->status_bar_mid_2;
        case STATUS_BAR_HIGH_1:
            return theme->status_bar_high_1;
        case STATUS_BAR_HIGH_2:
            return theme->status_bar_high_2;
        default:
            return 0;
    }
}
