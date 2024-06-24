/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "lauxlib.h"
#include "can.h"
#include "pdo.h"
#include "printf.h"
#include "table.h"

static pdo_t pdo[PDO_MAX];

static Uint32 pdo_send_callback(Uint32 interval, void *param);
static void   print_error(const char* reason, disp_mode_t disp_mode, Uint16 can_id);

SDL_bool pdo_add(Uint16 can_id, Uint32 event_time_ms, Uint8 length, Uint64 data, disp_mode_t disp_mode)
{
    int i;

    if (SDL_FALSE == pdo_is_id_valid(can_id))
    {
        print_error("Could not add PDO: Invalid TPDO CAN-ID", disp_mode, can_id);
        return SDL_FALSE;
    }

    if (length > 8)
    {
        length = 8;
    }

    if (SDL_FALSE == pdo_del(can_id, disp_mode))
    {
        /* Nothing to do here. */
    }

    for (i = 0; i < PDO_MAX; i += 1)
    {
        if (0 == pdo[i].can_id)
        {
            pdo[i].can_id = can_id;
            pdo[i].length = length;
            pdo[i].data   = data;
            pdo[i].id     = SDL_AddTimer(event_time_ms, pdo_send_callback, &pdo[i]);
            return SDL_TRUE;
        }
    }

    print_error("Could not add PDO: No empty slot available", disp_mode, can_id);
    return SDL_FALSE;
}

SDL_bool pdo_del(Uint16 can_id, disp_mode_t disp_mode)
{
    int i;

    if (SDL_FALSE == pdo_is_id_valid(can_id))
    {
        print_error("Could not delete PDO: Invalid TPDO CAN-ID", disp_mode, can_id);
        return SDL_FALSE;
    }

    for (i = 0; i < PDO_MAX; i += 1)
    {
        if (can_id == pdo[i].can_id)
        {
            pdo[i].can_id = 0;
            pdo[i].length = 0;
            pdo[i].data   = 0;

            SDL_RemoveTimer(pdo[i].id);
            break;
        }
    }

    return SDL_TRUE;
}

int lua_pdo_add(lua_State* L)
{
    int         can_id        = luaL_checkinteger(L, 1);
    int         event_time_ms = luaL_checkinteger(L, 2);
    int         length        = luaL_checkinteger(L, 3);
    Uint32      data_d0_d3    = lua_tointeger(L, 4);
    Uint32      data_d4_d7    = lua_tointeger(L, 5);
    SDL_bool    show_output   = lua_toboolean(L, 6);
    SDL_bool    success;
    const char* comment       = lua_tostring(L, 7);
    disp_mode_t disp_mode     = NO_OUTPUT;
    Uint64      data          = ((Uint64)data_d0_d3 << 32) | data_d4_d7;

    if (SDL_TRUE == show_output)
    {
        disp_mode = SCRIPT_OUTPUT;
    }

    success = pdo_add(can_id, event_time_ms, length, data, disp_mode);

    if (SDL_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        if (NULL == comment)
        {
            comment = "-";
        }

        SDL_strlcpy(buffer, comment, 33);
        for (i = SDL_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        if (SDL_TRUE == success)
        {
            c_printf(LIGHT_BLACK, "PDO  ");
            c_printf(DEFAULT_COLOR, "    0x%03X   -       -         -       ", can_id);
            c_printf(LIGHT_GREEN, "SUCC    ");
            c_printf(DARK_MAGENTA, "%s ", buffer);
            c_printf(DEFAULT_COLOR, "0x%08X%08X, %ums\n", data_d0_d3, data_d4_d7, event_time_ms);
        }
    }

    lua_pushboolean(L, success);

    return 1;
}

int lua_pdo_del(lua_State* L)
{
    int         can_id      = luaL_checkinteger(L, 1);
    SDL_bool    show_output = lua_toboolean(L, 2);
    const char* comment     = lua_tostring(L, 3);
    disp_mode_t disp_mode   = NO_OUTPUT;

    if (SDL_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        disp_mode = SCRIPT_OUTPUT;

        if (NULL == comment)
        {
            comment = "-";
        }

        SDL_strlcpy(buffer, comment, 33);
        for (i = SDL_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        c_printf(LIGHT_BLACK, "PDO  ");
        c_printf(DEFAULT_COLOR, "    0x%02X   -       -         -       ", can_id);
        c_printf(LIGHT_GREEN, "SUCC    ");
        c_printf(DEFAULT_COLOR, "%s ", buffer);
        c_printf(DEFAULT_COLOR, "Delete\n");
    }

    lua_pushboolean(L, pdo_del(can_id, disp_mode));
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
    int           i;
    int           offset  = 0;
    pdo_t*        pdo     = pdo_pt;
    can_message_t message = { 0 };

    message.id     = pdo->can_id;
    message.length = pdo->length;

    for (i = (pdo->length - 1); i >= 0; i -= 1)
    {
        message.data[i] = ((pdo->data >> offset) & 0xFF);
        offset += 8;
    }

    can_write(&message, NO_OUTPUT, NULL);

    return interval;
}

void pdo_print_help(void)
{
    table_t table = { DARK_CYAN, DARK_WHITE, 13, 7, 7 };

    table_print_header(&table);
    table_print_row("CAN-ID", "Object", "Spec.", &table);
    table_print_divider(&table);
    table_print_row("0x000 - 0x07f", "Node-ID", " ",       &table);
    table_print_row("0x181 - 0x1ff", "TPDO1",   "CiA 301", &table);
    table_print_row("0x281 - 0x1ff", "TPDO2",   "CiA 301", &table);
    table_print_row("0x381 - 0x1ff", "TPDO3",   "CiA 301", &table);
    table_print_row("0x481 - 0x1ff", "TPDO4",   "CiA 301", &table);
    table_print_footer(&table);
}

SDL_bool pdo_is_id_valid(Uint16 can_id)
{
    /* Node-ID (0x000 - 0x07f)
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

static void print_error(const char* reason, disp_mode_t disp_mode, Uint16 can_id)
{
    if (SCRIPT_OUTPUT != disp_mode)
    {
        return;
    }

    c_printf(LIGHT_BLACK, "PDO ");
    c_printf(DEFAULT_COLOR, "     0x%03X   -       -         -       ", can_id);
    c_printf(LIGHT_RED, "FAIL    ");
    c_printf(DEFAULT_COLOR, "%s\n", reason);
}
