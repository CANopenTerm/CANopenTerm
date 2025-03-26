/** @file table.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TABLE_H
#define TABLE_H

#include "os.h"

typedef struct table
{
    color_t frame_color;
    color_t text_color;
    uint8 column_a_width;
    uint8 column_b_width;
    uint8 column_c_width;

} table_t;

void table_flush(table_t* t);
status_t table_init(table_t* t, size_t initial_capacity);
void table_print_header(table_t* t);
void table_print_divider(table_t* t);
void table_print_footer(table_t* t);
void table_print_row(const char* column_a, const char* column_b, const char* column_c, table_t* t);

#endif /* TABLE_H */
