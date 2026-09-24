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

/* Theme palettes. */
static const pal_color_palette_t themes[] = {
    /* NEON_ECLIPSE */
    {
        .bg_color = 0x0b0d14,
        .widget_color = 0x171a25,
        .widget_color_highlight = 0x9b6cff,
        .draw_color = 0x00e6d0,
        .draw_white = 0xf4f6ff,
        .status_bar_low_1 = 0x00e5a8,
        .status_bar_low_2 = 0x007f8a,
        .status_bar_mid_1 = 0xffd84d,
        .status_bar_mid_2 = 0xff7a45,
        .status_bar_high_1 = 0xff4668,
        .status_bar_high_2 = 0xff00ff},
    /* OBSIDIAN_LUXE */
    {
        .bg_color = 0x101011,
        .widget_color = 0x1d1d20,
        .widget_color_highlight = 0xd4af37,
        .draw_color = 0xf2ede2,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x7bb661,
        .status_bar_low_2 = 0x3f7950,
        .status_bar_mid_1 = 0xe8c75a,
        .status_bar_mid_2 = 0xc77b3c,
        .status_bar_high_1 = 0xd94b4b,
        .status_bar_high_2 = 0xd4af37},
    /* DEEP_OCEAN */
    {
        .bg_color = 0x06141d,
        .widget_color = 0x0d2633,
        .widget_color_highlight = 0x249fca,
        .draw_color = 0x16d9ed,
        .draw_white = 0xe6f8fc,
        .status_bar_low_1 = 0x00d6b0,
        .status_bar_low_2 = 0x008b9a,
        .status_bar_mid_1 = 0xffdf58,
        .status_bar_mid_2 = 0xff9650,
        .status_bar_high_1 = 0xff526d,
        .status_bar_high_2 = 0x0a5f7a},
    /* EMBER_CORE */
    {
        .bg_color = 0x150d09,
        .widget_color = 0x261712,
        .widget_color_highlight = 0xf0643c,
        .draw_color = 0xffb86a,
        .draw_white = 0xfff3e7,
        .status_bar_low_1 = 0x7acb67,
        .status_bar_low_2 = 0x4a8f45,
        .status_bar_mid_1 = 0xffca45,
        .status_bar_mid_2 = 0xf17a32,
        .status_bar_high_1 = 0xff5145,
        .status_bar_high_2 = 0xff3d1a},
    /* TOXIC_LAB */
    {
        .bg_color = 0x090d08,
        .widget_color = 0x14200e,
        .widget_color_highlight = 0x74e62e,
        .draw_color = 0xaaf45d,
        .draw_white = 0xf1f9e8,
        .status_bar_low_1 = 0x83eb32,
        .status_bar_low_2 = 0x42b94a,
        .status_bar_mid_1 = 0xe8ed35,
        .status_bar_mid_2 = 0xff9a27,
        .status_bar_high_1 = 0xff4848,
        .status_bar_high_2 = 0x00ff00},
    /* ARCTIC_DAY */
    {
        .bg_color = 0xf1f6fa,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x1976c9,
        .draw_color = 0x075d68,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x36a078,
        .status_bar_low_2 = 0x4e9cc4,
        .status_bar_mid_1 = 0xf3b34c,
        .status_bar_mid_2 = 0xe87d62,
        .status_bar_high_1 = 0xd84c58,
        .status_bar_high_2 = 0x00b8e6},
    /* IVORY_LUXE */
    {
        .bg_color = 0xf7f3e9,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0xb28722,
        .draw_color = 0x403b35,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x6b9b45,
        .status_bar_low_2 = 0x72a989,
        .status_bar_mid_1 = 0xd7ad3f,
        .status_bar_mid_2 = 0xd88745,
        .status_bar_high_1 = 0xb94a43,
        .status_bar_high_2 = 0xc9985a},
    /* SKYLINE */
    {
        .bg_color = 0xe9f4fb,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x0785bd,
        .draw_color = 0x075589,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x159a91,
        .status_bar_low_2 = 0x3f9bc0,
        .status_bar_mid_1 = 0xf2a26b,
        .status_bar_mid_2 = 0xe97b69,
        .status_bar_high_1 = 0xc94a55,
        .status_bar_high_2 = 0x0066cc},
    /* SUNLIT_EMBER */
    {
        .bg_color = 0xfff6e9,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0xe15b18,
        .draw_color = 0xb53c13,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x6c9d45,
        .status_bar_low_2 = 0x8eb76c,
        .status_bar_mid_1 = 0xf5b34d,
        .status_bar_mid_2 = 0xef7f3d,
        .status_bar_high_1 = 0xd94d4d,
        .status_bar_high_2 = 0xff8c00},
    /* MINT_LAB */
    {
        .bg_color = 0xf0f7ef,
        .widget_color = 0xffffff,
        .widget_color_highlight = 0x568d3c,
        .draw_color = 0x356b27,
        .draw_white = 0xffffff,
        .status_bar_low_1 = 0x159b8d,
        .status_bar_low_2 = 0x4fae9c,
        .status_bar_mid_1 = 0xf0b83d,
        .status_bar_mid_2 = 0xe98935,
        .status_bar_high_1 = 0xd64d50,
        .status_bar_high_2 = 0x00cc99}
};

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
