/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "can.h"
#include "pdo.h"
#include "printf.h"

static pdo_t pdo[PDO_MAX];

static Uint32   pdo_send_callback(Uint32 interval, void *param);
static SDL_bool pdo_is_id_valid(Uint16 can_id);

void pdo_add(Uint16 can_id, Uint32 interval_in_ms, Uint8 length, Uint64 data)
{
    int index;

    // Check CAN-ID.
    if (SDL_FALSE == pdo_is_id_valid(can_id))
    {
        c_log(LOG_WARNING, "Invalid TPDO CAN-ID");
        return;
    }

    // Delete PDO to avoid duplicate entries.
    pdo_del(can_id);

    // Find empty slot.
    for (index = 0; index < PDO_MAX; index += 1)
    {
        if (0 == pdo[index].can_id)
        {
            pdo[index].can_id = can_id;
            pdo[index].length = length;
            pdo[index].data   = data;
            pdo[index].id     = SDL_AddTimer(interval_in_ms, pdo_send_callback, &pdo[index]);
            return;
        }
    }

    c_log(LOG_WARNING, "No empty PDO slot available");
}

void pdo_del(Uint16 can_id)
{
    int index;

    // Check CAN-ID.
    if (SDL_FALSE == pdo_is_id_valid(can_id))
    {
        c_log(LOG_WARNING, "Invalid TPDO CAN-ID");
        return;
    }

    for (index = 0; index < PDO_MAX; index += 1)
    {
        if (can_id == pdo[index].can_id)
        {
            pdo[index].can_id = 0;
            pdo[index].length = 0;
            pdo[index].data   = 0;

            SDL_RemoveTimer(pdo[index].id);
            return;
        }
    }
}

static Uint32 pdo_send_callback(Uint32 interval, void *pdo_pt)
{
    pdo_t*        pdo     = pdo_pt;
    can_message_t message = { 0 };

    message.id      = pdo->can_id;
    message.length  = pdo->length;

    message.data[7] = ( pdo->data        & 0xFF);
    message.data[6] = ((pdo->data >> 8)  & 0xFF);
    message.data[5] = ((pdo->data >> 16) & 0xFF);
    message.data[4] = ((pdo->data >> 24) & 0xFF);
    message.data[3] = ((pdo->data >> 32) & 0xFF);
    message.data[2] = ((pdo->data >> 40) & 0xFF);
    message.data[1] = ((pdo->data >> 48) & 0xFF);
    message.data[0] = ((pdo->data >> 56) & 0xFF);

    can_write(&message);

    return interval;
}

static SDL_bool pdo_is_id_valid(Uint16 can_id)
{
    /* TPDO 1 (0x181 - 0x1ff)
     * TPDO 2 (0x201 - 0x2ff)
     * TPDO 3 (0x381 - 0x3ff)
     * TPDO 4 (0x481 - 0x4ff)
     */
    if ((can_id >= 0x180) && (can_id <= 0x4ff))
    {
        switch (can_id)
        {
            case 0x180:
            case 0x280:
            case 0x380:
            case 0x480:
                return SDL_FALSE;
            default:
                return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}
