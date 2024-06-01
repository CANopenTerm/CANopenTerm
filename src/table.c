/** @file table.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "table.h"
#include "printf.h"

static void print_frame(const char* left, const char* center, const char* right, table_t* t);

void table_print_header(table_t* t)
{
    print_frame("┌─", "─╥─", "─┐", t);
}

void table_print_divider(table_t* t)
{
    print_frame("├─", "─╫─", "─┤", t);
}

void table_print_footer(table_t* t)
{
    print_frame("└─", "─╨─", "─┘", t);
}

void table_print_row(const char* column_a, const char* column_b, const char* column_c, table_t* t)
{
    int    index;
    size_t column_a_len = SDL_strlen(column_a);
    size_t column_b_len = SDL_strlen(column_b);
    size_t column_c_len = SDL_strlen(column_c);

    c_printf(t->frame_color, " │ ");

    for (index = 0; index < t->column_a_width; index += 1)
    {
        if (index <= (column_a_len - 1))
        {
            c_printf(t->text_color, "%c", column_a[index]);
        }
        else
        {
            c_printf(t->text_color, " ");
        }
    }
    c_printf(t->frame_color, " ║ ");

    for (index = 0; index < t->column_b_width; index += 1)
    {
        if (index <= (column_b_len - 1))
        {
            c_printf(t->text_color, "%c", column_b[index]);
        }
        else
        {
            c_printf(t->text_color, " ");
        }
    }
    c_printf(t->frame_color, " ║ ");

    for (index = 0; index < t->column_c_width; index += 1)
    {
        if (index <= (column_c_len - 1))
        {
            c_printf(t->text_color, "%c", column_c[index]);
        }
        else
        {
            c_printf(t->text_color, " ");
        }
    }
    c_printf(t->frame_color, " │\r\n");
}

static void print_frame(const char* left, const char* center, const char* right, table_t* t)
{
    int index;

    c_printf(t->frame_color, " %s", left);
    for (index = 0; index < t->column_a_width; index += 1)
    {
        c_printf(t->frame_color, "─");
    }
    c_printf(t->frame_color, "%s", center);

    for (index = 0; index < t->column_b_width; index += 1)
    {
        c_printf(t->frame_color, "─");
    }
    c_printf(t->frame_color, "%s", center);

    for (index = 0; index < t->column_c_width; index += 1)
    {
        c_printf(t->frame_color, "─");
    }
    c_printf(t->frame_color, "%s\r\n", right);
}
