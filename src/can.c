/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
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
