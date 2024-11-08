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
    int      i;
    int      status        = EXIT_SUCCESS;
    bool_t   is_silent     = IS_FALSE;
    core_t*  core          = NULL;
    char*    script        = NULL;
    char*    can_interface = DEFAULT_CAN_INTERFACE;

    for (i = 1; i < argc; i++)
    {
        if (0 == os_strcmp(argv[i], "-s") && (i + 1) < argc)
        {
            is_silent = IS_TRUE;
            script    = argv[++i];
        }
        else if (0 == os_strcmp(argv[i], "-i") && (i + 1) < argc)
        {
            can_interface = argv[++i];
        }
        else
        {
            os_printf("Usage: %s [-s script] [-i can_interface]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (core_init(&core, is_silent) != ALL_OK)
    {
        status = EXIT_FAILURE;
    }

    os_strlcpy(core->can_interface, can_interface, sizeof(core->can_interface));

    if (script != NULL)
    {
        run_script(script, core);
        core->is_running = IS_FALSE;
    }

    while (IS_TRUE == core->is_running)
    {
        switch (core_update(core))
        {
            case CORE_QUIT:
                core->is_running = IS_FALSE;
                continue;
            case OS_CONSOLE_INIT_ERROR:
            case OS_INIT_ERROR:
            case OS_MEMORY_ALLOCATION_ERROR:
            case SCRIPT_INIT_ERROR:
                status = EXIT_FAILURE;
                break;
            case ALL_OK:
            case CAN_NO_HARDWARE_FOUND:
            case CAN_READ_ERROR:
            case CAN_WRITE_ERROR:
            case EDS_OBJECT_NOT_AVAILABLE:
            case EDS_PARSE_ERROR:
            case ITEM_NOT_FOUND:
            case NMT_UNKNOWN_COMMAND:
            case NOTHING_TO_DO:
            case OS_FILE_NOT_FOUND:
            case OS_INVALID_ARGUMENT:
                continue;
            default:
                continue;
        }
    }

    core_deinit(core);
    return status;
}
