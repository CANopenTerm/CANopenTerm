/** @file can.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CAN_H
#define CAN_H

#include "SDL.h"
#include "core.h"

typedef struct can_message
{
    Uint8 length;
    Uint8 data[4];

} can_message_t;

void     can_init(core_t* core_t);
void     can_deinit(core_t* core);
SDL_bool is_can_initialised(core_t* core);

#endif /* CAN_H */
