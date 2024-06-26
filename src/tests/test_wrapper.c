/** @file test_wrapper.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "os.h"

uint32 __wrap_can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    uint32 status = 0;

    (void)disp_mode;
    (void)comment;

    if (NULL == message)
    {
        status = 1;
    }

    return status;
}
