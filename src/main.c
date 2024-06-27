/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>
#include "core.h"
#include "os.h"
#include "scripts.h"

int main(int argc, char* argv[])
{
    int      status = EXIT_SUCCESS;
    core_t*  core   = NULL;

    if (core_init(&core) != ALL_OK)
    {
        status = EXIT_FAILURE;
    }

    if ((argc > 2) && (argv[2] != NULL))
    {
        os_strlcpy(core->can_interface, argv[1], sizeof(core->can_interface));
    }
    else
    {
        os_strlcpy(core->can_interface, DEFAULT_CAN_INTERFACE, sizeof(core->can_interface));
    }

    if ((argc > 1) && (argv[1] != NULL))
    {
        run_script(argv[1], core);
        core->is_running = IS_FALSE;
    }

    while (IS_TRUE == core->is_running)
    {
        switch (core_update(core))
        {
            case CORE_QUIT:
                core->is_running = IS_FALSE;
                break;
            case OS_CONSOLE_INIT_ERROR:
            case OS_INIT_ERROR:
            case OS_MEMORY_ALLOCATION_ERROR:
            case SCRIPT_INIT_ERROR:
                status = EXIT_FAILURE;
                break;
            default:
                continue;
        }
    }

    core_deinit(core);
    return status;
}
