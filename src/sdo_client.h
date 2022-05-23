/** @file sdo_client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SDO_CLIENT_H
#define SDO_CLIENT_H

#include "SDL.h"
#include "core.h"

typedef enum
{
    UNSIGNED8  = 1,
    UNSIGNED16 = 2,
    UNSIGNED32 = 4

} data_type_t;

typedef enum
{
    READ_DICT_OBJECT = 0x40

} sdo_command_code_t;

Uint32 read_sdo(Uint8 node_id, Uint8 index, Uint16 sub_index);

#endif /* SDO_CLIENT_H */
