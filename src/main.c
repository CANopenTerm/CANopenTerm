/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef USE_LIBSOCKETCAN
#include <string.h>
#endif
#include <stdlib.h>
#include "core.h"
#include "SDL.h"
#include "SDL_main.h"

int main(int argc, char* argv[])
{
    int     status = EXIT_SUCCESS;
    core_t* core   = NULL;

    if (COT_OK != core_init(&core))
    {
        status = EXIT_FAILURE;
    }

#ifdef USE_LIBSOCKETCAN
    if ((argc > 1) && (argv[1] != NULL))
    {
        strncpy(core->can_interface, argv[1], sizeof(core->can_interface) - 1);
        core->can_interface[sizeof(core->can_interface) - 1] = '\0';
    }
    else
    {
        strcpy(core->can_interface, "can0");
    }
#endif

    while (SDL_TRUE == core->is_running)
    {
        switch (core_update(core))
        {
            case COT_QUIT:
                core->is_running = SDL_FALSE;
                break;
            case COT_ERROR:
                status = EXIT_FAILURE;
                break;
            default:
                continue;
        }
    }

    core_deinit(core);
    return status;
}
