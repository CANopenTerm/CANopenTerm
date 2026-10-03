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
    /* MIDNIGHT_BLUE */
    {
        .bg_color = 0x0b1220,
        .widget_color = 0x162033,
        .widget_color_highlight = 0x243653,
        .draw_color = 0x6ea8fe,
        .draw_color_alt = 0x7ee7d8,
        .status_bar_low_1 = 0x43d17d,
        .status_bar_low_2 = 0x62df91,
        .status_bar_mid_1 = 0xe0c85a,
        .status_bar_mid_2 = 0xf0a84b,
        .status_bar_high_1 = 0xf06a62,
        .status_bar_high_2 = 0xff5c67},
    /* DEEP_FOREST */
    {
        .bg_color = 0x0b1713,
        .widget_color = 0x13251e,
        .widget_color_highlight = 0x1e3a2f,
        .draw_color = 0x72d6a0,
        .draw_color_alt = 0x8ed8ff,
        .status_bar_low_1 = 0x48d98a,
        .status_bar_low_2 = 0x68e5a0,
        .status_bar_mid_1 = 0xd9d65d,
        .status_bar_mid_2 = 0xeeb04c,
        .status_bar_high_1 = 0xee7060,
        .status_bar_high_2 = 0xff5c63},
    /* DARK_PLUM */
    {
        .bg_color = 0x160f1c,
        .widget_color = 0x25172d,
        .widget_color_highlight = 0x392343,
        .draw_color = 0xd49af5,
        .draw_color_alt = 0xffa6c9,
        .status_bar_low_1 = 0x55d78b,
        .status_bar_low_2 = 0x72e59d,
        .status_bar_mid_1 = 0xe2ce63,
        .status_bar_mid_2 = 0xf0a85b,
        .status_bar_high_1 = 0xef6b78,
        .status_bar_high_2 = 0xff5c79},
    /* CHARCOAL_AMBER */
    {
        .bg_color = 0x121212,
        .widget_color = 0x1e1e1e,
        .widget_color_highlight = 0x303030,
        .draw_color = 0xffb454,
        .draw_color_alt = 0x62d9ff,
        .status_bar_low_1 = 0x52d77c,
        .status_bar_low_2 = 0x6ee58d,
        .status_bar_mid_1 = 0xe2cc51,
        .status_bar_mid_2 = 0xf1aa45,
        .status_bar_high_1 = 0xf06455,
        .status_bar_high_2 = 0xff5955},
    /* DARK_WINE */
    {
        .bg_color = 0x1a0d14,
        .widget_color = 0x2a151f,
        .widget_color_highlight = 0x422332,
        .draw_color = 0xf07891,
        .draw_color_alt = 0xb58cff,
        .status_bar_low_1 = 0x48d88a,
        .status_bar_low_2 = 0x68e59b,
        .status_bar_mid_1 = 0xe0c45c,
        .status_bar_mid_2 = 0xf0a04c,
        .status_bar_high_1 = 0xf06a73,
        .status_bar_high_2 = 0xff586d},
    /* DEERE_FIELD */
    {
        .bg_color = 0x10251a,
        .widget_color = 0x183523,
        .widget_color_highlight = 0x285038,
        .draw_color = 0x63c957,
        .draw_color_alt = 0xf4d23c,
        .status_bar_low_1 = 0x39c96b,
        .status_bar_low_2 = 0x5cdb7b,
        .status_bar_mid_1 = 0xd8cf45,
        .status_bar_mid_2 = 0xe8ad3d,
        .status_bar_high_1 = 0xe75d4f,
        .status_bar_high_2 = 0xf0444b},
    /* NEW_AMSTERDAM*/
    {
        .bg_color = 0x102438,
        .widget_color = 0x19334d,
        .widget_color_highlight = 0x285171,
        .draw_color = 0x4fa9e8,
        .draw_color_alt = 0xf3c84b,
        .status_bar_low_1 = 0x3bd084,
        .status_bar_low_2 = 0x5cde91,
        .status_bar_mid_1 = 0xd9cc4b,
        .status_bar_mid_2 = 0xe7aa3e,
        .status_bar_high_1 = 0xe85e55,
        .status_bar_high_2 = 0xf0474d},
    /* CRIMSON_HARVEST */
    {
        .bg_color = 0x261417,
        .widget_color = 0x351c20,
        .widget_color_highlight = 0x55282d,
        .draw_color = 0xf04f52,
        .draw_color_alt = 0xffc83d,
        .status_bar_low_1 = 0x3dca76,
        .status_bar_low_2 = 0x5ddd85,
        .status_bar_mid_1 = 0xe0c749,
        .status_bar_mid_2 = 0xeea43e,
        .status_bar_high_1 = 0xef6255,
        .status_bar_high_2 = 0xff4647},
    /* EMERALD_DRIVE */
    {
        .bg_color = 0x10231c,
        .widget_color = 0x183429,
        .widget_color_highlight = 0x28513d,
        .draw_color = 0x62c96b,
        .draw_color_alt = 0xe95757,
        .status_bar_low_1 = 0x3bd17a,
        .status_bar_low_2 = 0x5cde88,
        .status_bar_mid_1 = 0xd7cc4a,
        .status_bar_mid_2 = 0xe7a83e,
        .status_bar_high_1 = 0xe85b55,
        .status_bar_high_2 = 0xf0444b},    
    /* IRON_HARVEST */
    {
        .bg_color = 0x1c2022,
        .widget_color = 0x292e30,
        .widget_color_highlight = 0x41484a,
        .draw_color = 0xe6534d,
        .draw_color_alt = 0xf1c33c,
        .status_bar_low_1 = 0x42c978,
        .status_bar_low_2 = 0x60dc88,
        .status_bar_mid_1 = 0xd9c645,
        .status_bar_mid_2 = 0xe7a53b,
        .status_bar_high_1 = 0xe75b52,
        .status_bar_high_2 = 0xf04448
    },
    /* HYDRAULIC_AMBER */
    {
        .bg_color = 0x0c151d,
        .widget_color = 0x17232c,
        .widget_color_highlight = 0x293945,
        .draw_color = 0xf28c28,
        .draw_color_alt = 0x72b9d4,
        .status_bar_low_1 = 0x3fc978,
        .status_bar_low_2 = 0x64dc8d,
        .status_bar_mid_1 = 0xd8c54b,
        .status_bar_mid_2 = 0xeea443,
        .status_bar_high_1 = 0xe85e4f,
        .status_bar_high_2 = 0xff5149
    },
    /* SAFETY_SIGNAL */
    {
        .bg_color = 0x111514,
        .widget_color = 0x1d2422,
        .widget_color_highlight = 0x303a36,
        .draw_color = 0xffd23f,
        .draw_color_alt = 0xff7a32,
        .status_bar_low_1 = 0x4bd17b,
        .status_bar_low_2 = 0x6be28f,
        .status_bar_mid_1 = 0xe4c843,
        .status_bar_mid_2 = 0xf3a43b,
        .status_bar_high_1 = 0xef6248,
        .status_bar_high_2 = 0xff4542
    },
    /* SIGNAL_RED */
    {
        .bg_color = 0x101416,
        .widget_color = 0x1b2023,
        .widget_color_highlight = 0x30383c,
        .draw_color = 0xe83232,
        .draw_color_alt = 0x63b7df,
        .status_bar_low_1 = 0x43cc7b,
        .status_bar_low_2 = 0x65df91,
        .status_bar_mid_1 = 0xd7c84a,
        .status_bar_mid_2 = 0xeea342,
        .status_bar_high_1 = 0xe94d4b,
        .status_bar_high_2 = 0xff3f42
    },
    /* ELECTRIC_INDIGO */
    {
        .bg_color = 0x110f1d,
        .widget_color = 0x1d1830,
        .widget_color_highlight = 0x31274b,
        .draw_color = 0xa87cff,
        .draw_color_alt = 0x46d9e8,
        .status_bar_low_1 = 0x43d28a,
        .status_bar_low_2 = 0x68e3a0,
        .status_bar_mid_1 = 0xd8cb57,
        .status_bar_mid_2 = 0xeea84a,
        .status_bar_high_1 = 0xeb6078,
        .status_bar_high_2 = 0xff4e69
    },
    /* BUS_SIGNAL */
    {
        .bg_color = 0x0b1821,
        .widget_color = 0x142631,
        .widget_color_highlight = 0x213c4b,
        .draw_color = 0x38c9e6,
        .draw_color_alt = 0x7cdb73,
        .status_bar_low_1 = 0x3dce83,
        .status_bar_low_2 = 0x61df98,
        .status_bar_mid_1 = 0xd5ca4d,
        .status_bar_mid_2 = 0xe9a844,
        .status_bar_high_1 = 0xe75f59,
        .status_bar_high_2 = 0xff4b55
    }
};

/* Current theme (default to MIDNIGHT_BLUE) */
static pal_theme_t current_theme = MIDNIGHT_BLUE;

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
    if (theme >= MIDNIGHT_BLUE && theme <= BUS_SIGNAL)
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
        case DRAW_COLOR_ALT:
            return theme->draw_color_alt;
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
