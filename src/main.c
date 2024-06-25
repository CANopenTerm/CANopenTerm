/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef __linux__
#include <string.h>
#endif
#include <stdlib.h>
#include "core.h"
#include "os.h"
#include "scripts.h"

int main(int argc, char* argv[])
{
    int      status      = EXIT_SUCCESS;
    int      script_arg  = 1;
    core_t*  core        = NULL;

    if (core_init(&core) != ALL_OK)
    {
        status = EXIT_FAILURE;
    }

#ifdef __linux__
    if ((argc > 1) && (argv[1] != NULL))
    {
        os_strlcpy(core->can_interface, argv[1], sizeof(core->can_interface));
    }
    else
    {
        os_strlcpy(core->can_interface, "can0", sizeof(core->can_interface));
    }
    script_arg = 2;
#endif

    if ((argc > script_arg) && (argv[script_arg] != NULL))
    {
        run_script(argv[script_arg], core);
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

