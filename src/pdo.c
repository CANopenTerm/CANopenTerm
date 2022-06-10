/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "can.h"
#include "pdo.h"
#include "printf.h"
#include "table.h"

static pdo_t pdo[PDO_MAX];

static Uint32 pdo_send_callback(Uint32 interval, void *param);

void pdo_add(Uint16 can_id, Uint32 event_time_ms, Uint8 length, Uint64 data)
{
    int index;

    // Check CAN-ID.
    if (SDL_FALSE == pdo_is_id_valid(can_id))
    {
        c_log(LOG_WARNING, "Invalid TPDO CAN-ID");
        return;
    }

    if (length > 8)
    {
        length = 8;
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
            pdo[index].id     = SDL_AddTimer(event_time_ms, pdo_send_callback, &pdo[index]);
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

int lua_pdo_add(lua_State* L)
{
    int    can_id        = luaL_checkinteger(L, 1);
    int    event_time_ms = luaL_checkinteger(L, 2);
    int    length        = luaL_checkinteger(L, 3);
    Uint32 data_d0_d3    = luaL_checkinteger(L, 4);
    Uint32 data_d4_d7    = luaL_checkinteger(L, 5);
    Uint64 data          = ((Uint64)data_d0_d3 << 32) | data_d4_d7;

    pdo_add(can_id, event_time_ms, length, data);

    return 1;
}

int lua_pdo_del(lua_State* L)
{
    int can_id = luaL_checkinteger(L, 1);

    pdo_del(can_id);

    return 1;
}

void lua_register_pdo_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_pdo_add);
    lua_setglobal(core->L, "pdo_add");

    lua_pushcfunction(core->L, lua_pdo_del);
    lua_setglobal(core->L, "pdo_del");
}

static Uint32 pdo_send_callback(Uint32 interval, void *pdo_pt)
{
    int           index;
    int           offset  = 0;
    pdo_t*        pdo     = pdo_pt;
    can_message_t message = { 0 };

    message.id     = pdo->can_id;
    message.length = pdo->length;

    for (index = (pdo->length - 1); index >= 0; index -= 1)
    {
        message.data[index] = ((pdo->data >> offset) & 0xFF);
        offset += 8;
    }

    can_write(&message);

    return interval;
}

void pdo_print_help(void)
{
    table_t table = { DARK_CYAN, DARK_WHITE, 13, 7, 7 };

    table_print_header(&table);
    table_print_row("CAN-ID", "Object", "Spec.", &table);
    table_print_divider(&table);
    table_print_row("0x00  - 0x7f",  "Node-ID", "CiA 301", &table);
    table_print_row("0x181 - 0x1ff", " TPDO1",  "CiA 301", &table);
    table_print_row("0x281 - 0x1ff", " TPDO2",  "CiA 301", &table);
    table_print_row("0x381 - 0x1ff", " TPDO3",  "CiA 301", &table);
    table_print_row("0x481 - 0x1ff", " TPDO4",  "CiA 301", &table);
    table_print_footer(&table);
}

SDL_bool pdo_is_id_valid(Uint16 can_id)
{
    /* Node-ID (0x00  - 0x7f)
     * TPDO 1  (0x181 - 0x1ff)
     * TPDO 2  (0x281 - 0x2ff)
     * TPDO 3  (0x381 - 0x3ff)
     * TPDO 4  (0x481 - 0x4ff)
     */
    if ((can_id >= 0x00) && (can_id <= 0x7f))
    {
        return SDL_TRUE;
    }    
    else if ((can_id >= 0x180) && (can_id <= 0x4ff))
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
