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

Uint32 write_sdo(Uint8 index, Uint16 sub_index, data_type_t data_type, Uint32 data, Uint8 node_id);

#endif /* SDO_CLIENT_H */
