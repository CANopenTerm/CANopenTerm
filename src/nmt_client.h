/** @file nmt_client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef NMT_CLIENT_H
#define NMT_CLIENT_H

#include "SDL.h"
#include "core.h"

typedef enum
{
    NMT_OPERATIONAL     = 0x01,
    NMT_STOP            = 0x02,
    NMT_PRE_OPERATIONAL = 0x80,
    NMT_RESET_NODE      = 0x81,
    NMT_RESET_COMM      = 0x82

} nmt_command_t;

Uint32 send_nmt_command(nmt_command_t command, Uint8 node_id);
void   nmt_client_widget(core_t* core);

#endif /* NMT_CLIENT_H */
