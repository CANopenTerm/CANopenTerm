/** @file oscilloscope.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include "os.h"

#define OSCILLOSCOPE_BUFFER_SIZE 512

typedef struct
{
    uint32 buffer[OSCILLOSCOPE_BUFFER_SIZE];
    uint32 head;
    uint32 tail;
    uint32 size;
    uint32 max_value;
    uint32 min_value;

} oscilloscope_ringbuffer_t;

oscilloscope_ringbuffer_t* oscilloscope_buffer_create(uint32 min_value, uint32 max_value);
void oscilloscope_buffer_destroy(oscilloscope_ringbuffer_t* buffer);
void oscilloscope_buffer_push(oscilloscope_ringbuffer_t* buffer, uint32 value);
uint32 oscilloscope_buffer_get(oscilloscope_ringbuffer_t* buffer, uint32 index);
uint32 oscilloscope_buffer_get_size(oscilloscope_ringbuffer_t* buffer);

void widget_oscilloscope(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer, uint32 current_value, const char* label);

#endif /* OSCILLOSCOPE_H */
