/** @file can.c
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
#include "core.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "PCANBasic.h"

static int can_monitor(void *core);

void can_init(core_t* core)
{
    core->can_monitor_th = SDL_CreateThread(can_monitor, "CAN monitor thread", (void *)core);
}

void can_deinit(core_t* core)
{
    core->can_status         = 0;
    core->is_can_initialised = SDL_FALSE;

    CAN_Uninitialize(PCAN_NONEBUS);
    SDL_DetachThread(core->can_monitor_th);
}

Uint32 can_write(can_message_t* message)
{
    int      index;
    TPCANMsg pcan_message = { 0 };

    pcan_message.ID      = message->id;
    pcan_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    pcan_message.LEN     = message->length;

    for (index = 0; index < 8; index += 1)
    {
        pcan_message.DATA[index] = message->data[index];
    }

    return (Uint32)CAN_Write(PCAN_USBBUS1, &pcan_message);
}

Uint32 can_read(can_message_t* message)
{
    int      index;
    Uint32   can_status;
    TPCANMsg pcan_message = { 0 };

    can_status = CAN_Read(PCAN_USBBUS1, &pcan_message, NULL);

    message->id     = pcan_message.ID;
    message->length = pcan_message.LEN;

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = pcan_message.DATA[index];
    }

    return can_status;
}

int lua_can_write(lua_State* L)
{
    int    can_id         = luaL_checkinteger(L, 1);
    int    data_length    = luaL_checkinteger(L, 2);
    Uint64 data_d0_d3     = luaL_checkinteger(L, 3);
    Uint64 data_d4_d7     = luaL_checkinteger(L, 4);
    can_message_t message = { 0 };

    message.id     = can_id;
    message.length = data_length;

    message.data[3] = ( data_d0_d3        & 0xFF);
    message.data[2] = ((data_d0_d3 >> 8)  & 0xFF);
    message.data[1] = ((data_d0_d3 >> 16) & 0xFF);
    message.data[0] = ((data_d0_d3 >> 24) & 0xFF);
    message.data[7] = ( data_d4_d7        & 0xFF);
    message.data[6] = ((data_d4_d7 >> 8)  & 0xFF);
    message.data[5] = ((data_d4_d7 >> 16) & 0xFF);
    message.data[4] = ((data_d4_d7 >> 24) & 0xFF);

    if (0 != can_write(&message))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void lua_register_can_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_can_write);
    lua_setglobal(core->L, "can_write");
}

void can_print_error_message(const char* context, Uint32 can_status)
{
    char err_message[100] = { 0 };
    if (PCAN_ERROR_OK != can_status)
    {
        CAN_GetErrorText(can_status, 0x09, err_message);
        if (NULL == context)
        {
            SDL_LogWarn(0, "%s", err_message);
        }
        else
        {
            SDL_LogWarn(0, "%s: %s", context, err_message);
        }
    }
}

SDL_bool is_can_initialised(core_t* core)
{
    if (NULL == core)
    {
        return SDL_FALSE;
    }

    return core->is_can_initialised;
}

static int can_monitor(void *core_pt)
{
    char    err_message[100] = { 0 };
    core_t* core             = core_pt;

    if (NULL == core)
    {
        return 1;
    }

    while (SDL_TRUE == core->is_running)
    {
        while (SDL_FALSE == is_can_initialised(core))
        {
            core->can_status = CAN_Initialize(
                PCAN_USBBUS1,
                PCAN_BAUD_250K,
                PCAN_USB,
                0, 0);

            CAN_GetErrorText(core->can_status, 0x09, err_message);

            if ((core->can_status & PCAN_ERROR_OK) == core->can_status)
            {
                SDL_LogInfo(0, "CAN successfully initialised");
                core->is_can_initialised = SDL_TRUE;
            }

            SDL_Delay(100);
            continue;
        }

        core->can_status = CAN_GetStatus(PCAN_USBBUS1);

        if (PCAN_ERROR_ILLHW == core->can_status)
        {
            core->can_status         = 0;
            core->is_can_initialised = SDL_FALSE;

            CAN_Uninitialize(PCAN_NONEBUS);
            SDL_LogWarn(0, "CAN de-initialised: USB-dongle removed?");
        }
        SDL_Delay(100);
    }

    return 0;
}
