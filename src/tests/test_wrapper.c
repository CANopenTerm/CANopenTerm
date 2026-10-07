/** @file test_wrapper.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "os.h"

#include "can.h"
#include "os.h"

/* Mock wrapper for os_kbhit() - returns 0 (no key) in test environment */
int __wrap_os_kbhit(void)
{
    return 0;
}

uint32 __wrap_can_read(can_message_t* message, disp_mode_t disp_mode, const char* comment)
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
