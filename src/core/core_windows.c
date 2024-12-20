/** @file core_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <windows.h>

#include "core.h"
#include "os.h"

BOOL WINAPI handle_ctrl_c(DWORD signal)
{
    extern core_t* core;

    if (signal == CTRL_C_EVENT)
    {
        if (core != NULL)
        {
            printf("\r");
            os_log(LOG_INFO, "Ctrl+C pressed. Cleaning up...");
            fflush(stdout);
            core_deinit(core);
        }
        exit(EXIT_SUCCESS);
    }
    return TRUE;
}

void core_register_ctrl_c_handler(void)
{
    SetConsoleCtrlHandler(handle_ctrl_c, TRUE);
}
