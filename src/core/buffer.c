/** @file buffer.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "buffer.h"
#include "os.h"

static buffer_t buffer = {NULL, 0, 0, IS_FALSE};

void buffer_free()
{
    os_free(buffer.buffer);

    buffer.buffer     = NULL;
    buffer.size       = 0;
    buffer.capacity   = 0;
    buffer.use_buffer = IS_FALSE;
}

status_t buffer_init(size_t initial_capacity)
{
    status_t status = ALL_OK;

    buffer.buffer = (char*)os_calloc(initial_capacity, sizeof(char));
    if (NULL == buffer.buffer)
    {
        status = OS_MEMORY_ALLOCATION_ERROR;
    }
    else
    {
        buffer.size       = 0;
        buffer.capacity   = initial_capacity;
        buffer.use_buffer = IS_TRUE;
    }

    return status;
}

void buffer_write(const char* format, ...)
{
    int required;

    va_list_t args;
    os_va_start(args, format);

    required = os_vsnprintf(NULL, 0, format, args) + 1;
    os_va_end(args);

    if (buffer.size + required > buffer.capacity)
    {
        buffer.capacity = buffer.size + required + 1024;
        buffer.buffer   = (char*)os_realloc(buffer.buffer, buffer.capacity);
    }

    os_va_start(args, format);
    os_vsnprintf(buffer.buffer + buffer.size, required, format, args);
    os_va_end(args);

    buffer.size += required - 1;
}

void buffer_flush()
{
    fwrite(buffer.buffer, 1, buffer.size, stdout);
    buffer.size = 0;
}

bool_t use_buffer(void)
{
    return buffer.use_buffer;
}
