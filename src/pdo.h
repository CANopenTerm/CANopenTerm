/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PDO_H
#define PDO_H

#include "SDL.h"

#define PDO_MAX 0x1f8 // TPDO1 - TPDO4

typedef struct pdo
{
    SDL_TimerID id;
    Uint16      can_id;
    Uint8       length;
    Uint64      data;

} pdo_t;

void pdo_add(Uint16 can_id, Uint32 interval_in_ms, Uint8 length, Uint64 data);
void pdo_del(Uint16 can_id);

#endif /* PDO_H */
