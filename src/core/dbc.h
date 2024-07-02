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

#include "os.h"

typedef struct
{
    char *name;
    int   start_bit;
    int   length;
    float scale;
    float offset;
    float min_value;
    float max_value;
    char* unit;
    char* receiver;

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

const char* dbc_decode(dbc_t* dbc, uint16 id, uint32 data_d0_d3, uint32 data_d4_d7);
void        dbc_deinit(dbc_t* dbc);
status_t    dbc_init(dbc_t **dbc);
status_t    dbc_load(dbc_t *dbc, const char *filename);
void        dbc_print(const dbc_t *dbc);

#endif /* DBC_H */
