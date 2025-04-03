/** @file core_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <windows.h>

#include "core.h"
#include "os.h"

BOOL WINAPI handle_ctrl_c(DWORD signal)
{
    extern core_t* core;

    if (CTRL_C_EVENT == signal)
    {
        if (core != NULL)
        {
            printf("\r");
            os_log(LOG_INFO, "Ctrl+C pressed. Cleaning up...");
            fflush(stdout);
            os_detach_thread(core->core_th);
            core_deinit(core);
        }
        exit(EXIT_SUCCESS);
    }
    else if (CTRL_CLOSE_EVENT == signal)
    {
        if (core != NULL)
        {
            os_detach_thread(core->core_th);
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
