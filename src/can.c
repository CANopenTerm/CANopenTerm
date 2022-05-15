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

void init_can(core_t* core)
{
    char err_message[100] = { 0 };

    core->can_status = CAN_Initialize(
        PCAN_USBBUS1,
        PCAN_BAUD_250K,
        PCAN_USB,
        0, 0);

    CAN_GetErrorText(core->can_status, 0x09, err_message);

    if ((core->can_status & PCAN_ERROR_OK) == core->can_status)
    {
        SDL_LogInfo(0, "CAN successfully initialised");
        core->can_initialised = SDL_TRUE;
    }
    else
    {
        SDL_LogVerbose(0, "CAN could not be initialised: %s", err_message);
        core->can_initialised = SDL_FALSE;
    }
}

void deinit_can(core_t* core)
{
    core->can_status      = 0;
    core->can_initialised = SDL_FALSE;
    CAN_Uninitialize(PCAN_NONEBUS);
}

SDL_bool is_can_initialised(core_t* core)
{
    return core->can_initialised;
}
