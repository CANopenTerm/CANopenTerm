/** @file dbc.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef DBC_H
#define DBC_H

#include "core.h"
#include "os.h"

typedef enum
{
    ENDIANNESS_MOTOROLA = 0,
    ENDIANNESS_INTEL

} endian_t;

typedef struct
{
    char*    name;
    int      start_bit;
    int      length;
    endian_t endianness;
    float    scale;
    float    offset;
    float    min_value;
    float    max_value;
    char*    unit;
    char*    receiver;

} signal_t;

typedef struct
{
    char*        name;
    unsigned int id;
    unsigned int dlc;
    char*        transmitter;
    int          signal_count;
    signal_t*    signals;

} message_t;

typedef struct
{
    int        message_count;
    message_t* messages;

} dbc_t;

const char* dbc_decode(uint32 can_id, uint64 data);
status_t    dbc_find_id_by_name(uint32* id, const char* search);
status_t    dbc_load(char* filename);
void        dbc_print(void);
void        dbc_unload(void);

#endif /* DBC_H */
