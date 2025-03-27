/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL3/SDL.h>
#include <stdlib.h>

#include "can.h"
#include "codb.h"
#include "core.h"
#include "ctt.h"
#include "os.h"
#include "scripts.h"
#include "tachometer.h"
#include "window.h"

core_t* core = NULL;

int main(int argc, char* argv[])
{
    bool is_plain_mode = false;
    bool run_cct = false;
    char* can_interface = DEFAULT_CAN_INTERFACE;
    char* script = NULL;
    int status = EXIT_SUCCESS;
    uint32 node_id = 0x01;
    uint8 baud_rate_index = 0;

    core_register_ctrl_c_handler();

    if (argc > 1 && (argv[1][0] != '-') && (2 == argc))
    {
        script = argv[1];
        is_plain_mode = true;
    }
    else
    {
        int i;

        for (i = 1; i < argc; i++)
        {
            if (0 == os_strcmp(argv[i], "-s") && (i + 1) < argc)
            {
                script = argv[++i];
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

                node_id = (uint32)os_strtol(argv[++i], &endptr, 0);
                if (node_id < 1 || node_id > 127 || *endptr != '\0')
                {
                    os_printf("Invalid node ID.  Must be between 0x01 and 0x7F.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == os_strcmp(argv[i], "-p"))
            {
                is_plain_mode = true;
            }
            else if (0 == os_strcmp(argv[i], "-t"))
            {
                run_cct = true;
                is_plain_mode = true;
            }
            else
            {
                os_printf("Usage: %s [OPTION]\n\n", argv[0]);
                os_printf("    SCRIPT            Run script, implies -p\n");
                os_printf("                      Can't be combined with other options\n\n");
                os_printf("    -s SCRIPT         Run script (.lua can be ommited)\n");
                os_printf("    -i INTERFACE      Set CAN interface\n");
                os_printf("    -b BAUD           Set baud rate\n");
                os_printf("                        0 = 1 MBit/s\n");
                os_printf("                        2 = 500 kBit/s\n");
                os_printf("                        3 = 250 kBit/s\n");
                os_printf("                        4 = 125 kBit/s\n");
                os_printf("    -n NODE_ID        Set node ID, default: 0x01\n");
                os_printf("    -p                Run in plain mode\n");
                os_printf("    -t                Run conformance test, implies -p\n");
                /* os_printf("    -e EDS_FILE       Test EDS file, implies -t\n"); */

                exit(EXIT_FAILURE);
            }
        }
    }

    if (core_init(&core, is_plain_mode) != ALL_OK)
    {
        status = EXIT_FAILURE;
    }

    os_strlcpy(core->can_interface, can_interface, sizeof(core->can_interface));
    if (baud_rate_index != 0)
    {
        while (false == is_can_initialised(core)) {}
        can_set_baud_rate(baud_rate_index, core);
    }

    if (! window_init(core))
    {
        /* Nothing to do here. */
    }

    if (true == run_cct)
    {
        cct_run_test(node_id);
        core->is_running = false;
    }
    else if (script != NULL)
    {
        run_script(script, core);
        core->is_running = false;
    }

    core->core_th = os_create_thread(core_update, "Core thread", (void*)core);
    while (true == core->is_running)
    {
        while (SDL_PollEvent(&core->user_event))
        {
            switch (core->user_event.type)
            {
                case SDL_EVENT_USER:
                {
                    if (RUN_SCRIPT_EVENT == core->user_event.user.code)
                    {
                        run_script(core->user_event.user.data1, core);
                    }
                    break;
                }
            }
        }

        window_update();
        os_delay(1);
    }
    os_detach_thread(core->core_th);

    window_deinit(core);
    core_deinit(core);
    os_quit();
    return status;
}
