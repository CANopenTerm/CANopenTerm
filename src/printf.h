/** @file printf.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

typedef enum color
{
    DARK_BLACK = 0,
    DARK_BLUE,
    DARK_GREEN,
    DARK_CYAN,
    DARK_RED,
    DARK_MAGENTA,
    DARK_YELLOW,
    DARK_WHITE,
    LIGHT_BLACK,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_YELLOW,
    LIGHT_WHITE

} color_t;

typedef enum log_level
{
    LOG_DEFAULT = 0,
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARNING,
    LOG_ERROR

} log_level_t;

void c_printf(const color_t color, const char* format, ...);
void c_log(const log_level_t level, const char* format, ...);

#endif /* PRINTF_H */
