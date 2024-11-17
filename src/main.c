/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>
#include "can.h"
#include "core.h"
#include "eds.h"
#include "os.h"
#include "scripts.h"

core_t* core = NULL;

int main(int argc, char* argv[])
{
    bool_t   is_plain_mode   = IS_FALSE;
    char*    can_interface   = DEFAULT_CAN_INTERFACE;
    char*    eds_file        = NULL;
    char*    script          = NULL;
    int      i;
    int      status          = EXIT_SUCCESS;
    uint32   node_id         = 0x01;
    uint8    baud_rate_index = 0;

    core_register_ctrl_c_handler();

    for (i = 1; i < argc; i++)
    {
        if (0 == os_strcmp(argv[i], "-s") && (i + 1) < argc)
        {
            script = argv[++i];
        }
        else if (0 == os_strcmp(argv[i], "-t"))
        {
            script        = NULL;
            eds_file      = argv[++i];
            is_plain_mode = IS_TRUE;
        }
        else if (0 == os_strcmp(argv[i], "-i") && (i + 1) < argc)
        {
            can_interface = argv[++i];
        }
        else if (0 == os_strcmp(argv[i], "-b") && (i + 1) < argc)
        {
            char* endptr;

            baud_rate_index = (uint8)os_strtol(argv[++i], &endptr, 0);
            if (baud_rate_index > 13 || *endptr != '\0')
            {
                os_printf("Invalid baud rate.  Must be between 0 and 13.\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (0 == os_strcmp(argv[i], "-n") && (i + 1) < argc)
        {
            char* endptr;

            node_id = os_strtol(argv[++i], &endptr, 0);
            if (node_id < 1 || node_id > 127 || *endptr != '\0')
            {
                os_printf("Invalid node ID.  Must be between 0x01 and 0x7F.\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (0 == os_strcmp(argv[i], "-p"))
        {
            is_plain_mode = IS_TRUE;
        }
        else
        {
            os_printf("Usage: %s [OPTION]\n\n", argv[0]);
            os_printf("    -s SCRIPT         Run script (.lua can be ommited)\n");
            os_printf("    -t EDS            Run EDS conformance test (implies -p)\n");
            os_printf("    -i INTERFACE      Set CAN interface\n");
            os_printf("    -b BAUD           Set baud rate\n");
            os_printf("                        0 = 1 MBit/s\n");
            os_printf("                        2 = 500 kBit/s\n");
            os_printf("                        3 = 250 kBit/s\n");
            os_printf("                        4 = 125 kBit/s\n");
            os_printf("    -n NODE_ID        Set node ID, default: 0x01\n");
            os_printf("    -p                Run in plain mode\n");
            exit(EXIT_FAILURE);
        }
    }

    if (core_init(&core, is_plain_mode) != ALL_OK)
    {
        status = EXIT_FAILURE;
    }

    os_strlcpy(core->can_interface, can_interface, sizeof(core->can_interface));
    if (baud_rate_index != 0)
    {
        while (IS_FALSE == is_can_initialised(core)) {}
        can_set_baud_rate(baud_rate_index, core);
    }

    if (script != NULL)
    {
        run_script(script, core);
        core->is_running = IS_FALSE;
    }
    else if (eds_file != NULL)
    {
        run_conformance_test(eds_file, node_id);
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
            case SCRIPT_ERROR:
                continue;
            default:
                continue;
        }
    }

    core_deinit(core);
    return status;
}
