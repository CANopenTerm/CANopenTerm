/** @file table.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "table.h"
#include "buffer.h"
#include "os.h"

static void print_frame(const char* left, const char* center, const char* right, table_t* t);

void table_flush(table_t* t)
{
    buffer_flush();
    buffer_free();
}

status_t table_init(table_t* t, size_t initial_capacity)
{
    return buffer_init(initial_capacity);
}

void table_print_header(table_t* t)
{
    print_frame("\n ┌─", "─╥─", "─┐", t);
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
    size_t column_a_len = os_strlen(column_a);
    size_t column_b_len = os_strlen(column_b);
    size_t column_c_len = os_strlen(column_c);

    os_print(t->frame_color, " │ ");

    for (index = 0; index < t->column_a_width; index += 1)
    {
        if (index <= (column_a_len - 1))
        {
            os_print(t->text_color, "%c", column_a[index]);
        }
        else
        {
            os_print(t->text_color, " ");
        }
    }
    os_print(t->frame_color, " ║ ");

    for (index = 0; index < t->column_b_width; index += 1)
    {
        if (index <= (column_b_len - 1))
        {
            os_print(t->text_color, "%c", column_b[index]);
        }
        else
        {
            os_print(t->text_color, " ");
        }
    }
    os_print(t->frame_color, " ║ ");

    for (index = 0; index < t->column_c_width; index += 1)
    {
        if (index <= (column_c_len - 1))
        {
            os_print(t->text_color, "%c", column_c[index]);
        }
        else
        {
            os_print(t->text_color, " ");
        }
    }
    os_print(t->frame_color, " │\r\n");
}

static void print_frame(const char* left, const char* center, const char* right, table_t* t)
{
    int index;

    os_print(t->frame_color, " %s", left);
    for (index = 0; index < t->column_a_width; index += 1)
    {
        os_print(t->frame_color, "─");
    }
    os_print(t->frame_color, "%s", center);

    for (index = 0; index < t->column_b_width; index += 1)
    {
        os_print(t->frame_color, "─");
    }
    os_print(t->frame_color, "%s", center);

    for (index = 0; index < t->column_c_width; index += 1)
    {
        os_print(t->frame_color, "─");
    }
    os_print(t->frame_color, "%s\r\n", right);
}
