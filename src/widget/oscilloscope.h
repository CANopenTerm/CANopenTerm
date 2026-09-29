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

typedef struct
{
    uint32* buffer;
    uint64* timestamps;
    uint32 head;
    uint32 tail;
    uint32 size;
    uint32 capacity;
    uint32 max_value;
    uint32 min_value;

} oscilloscope_ringbuffer_t;

oscilloscope_ringbuffer_t* oscilloscope_buffer_create(uint32 min_value, uint32 max_value, uint32 size);
void oscilloscope_buffer_destroy(oscilloscope_ringbuffer_t* buffer);
void oscilloscope_buffer_push(oscilloscope_ringbuffer_t* buffer, uint32 value);
uint32 oscilloscope_buffer_get(oscilloscope_ringbuffer_t* buffer, uint32 index);
uint32 oscilloscope_buffer_get_size(oscilloscope_ringbuffer_t* buffer);

void widget_oscilloscope(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer, uint32 current_value, const char* label, uint64 time_window_ms);
void widget_oscilloscope_2ch(uint32 pos_x, uint32 pos_y, uint32 width, uint32 height, oscilloscope_ringbuffer_t* buffer1, uint32 current_value1, const char* label1, oscilloscope_ringbuffer_t* buffer2, uint32 current_value2, const char* label2, uint64 time_window_ms);

#endif /* OSCILLOSCOPE_H */
