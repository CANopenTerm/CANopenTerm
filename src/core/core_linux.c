/** @file core_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <signal.h>
#include <stdlib.h>

#include "core.h"
#include "os.h"

void handle_sigint(int sig)
{
    extern core_t* core;

    (void)sig;
    os_log(LOG_INFO, "Ctrl+C pressed. Cleaning up...");
    if (core != NULL)
    {
        core_deinit(core);
    }
    exit(EXIT_SUCCESS);
}

void core_register_ctrl_c_handler(void)
{
    signal(SIGINT, handle_sigint);
}
