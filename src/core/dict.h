/** @file dict.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef DICT_H
#define DICT_H

#include "can.h"
#include "core.h"
#include "os.h"

typedef struct dict_entry
{
    uint16      index_start;
    uint16      index_end;
    uint8       sub_index_start;
    uint8       sub_index_end;
    const char* description;

} dict_entry_t;

const char* dict_lookup(uint16 index, uint8 sub_index);
const char* dict_lookup_raw(can_message_t* message);

#endif /* DICT_H */
