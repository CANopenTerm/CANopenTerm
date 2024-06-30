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

typedef struct
{
    char* name;
    int   tart_bit;
    int   length;
    float scale;
    float offset;
    char* unit;
    float min_value;
    float max_value;
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

dbc_t *dbc_load(const char *filename);

#endif /* DBC_H */
