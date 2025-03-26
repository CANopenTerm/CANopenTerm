/** @file dict.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef DICT_H
#define DICT_H

#include "can.h"
#include "core.h"
#include "os.h"

typedef struct emcy_entry
{
    uint16 code;
    const char* description;

} emcy_entry_t;

const char* dict_lookup(uint16 index, uint8 sub_index);
status_t dict_lookup_object(uint16 index, uint8 sub_index);
const char* dict_lookup_raw(can_message_t* message);
const char* emcy_lookup(uint16 code);

#endif /* DICT_H */
