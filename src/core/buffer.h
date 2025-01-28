/** @file buffer.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef BUFFER_H
#define BUFFER_H

#include "os.h"

typedef struct
{
    char*  buffer;
    size_t size;
    size_t capacity;
    bool_t use_buffer;

} buffer_t;

void     buffer_flush();
void     buffer_free();
status_t buffer_init(size_t initial_capacity);
void     buffer_write(const char* format, ...);
bool_t   use_buffer(void);

#endif /* BUFFER_H */
