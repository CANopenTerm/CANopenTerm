/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>

#include "can.h"
#include "codb.h"
#include "core.h"
#include "ctt.h"
#include "os.h"
#include "scripts.h"
#include "sdo.h"
#include "tachometer.h"
#include "window.h"

core_t* core = NULL;

int main(int argc, char* argv[])
{
    bool is_plain_mode = false;
    bool run_cct = false;
    bool fullscreen = false;
    char* can_interface = DEFAULT_CAN_INTERFACE;
    char* script = NULL;
    char* db_file = NULL;
    char* report_file = NULL;
    char* eds_file = NULL;
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
                if (baud_rate_index > 14 || *endptr != '\0')
                {
                    os_printf("Invalid baud rate.  Must be between 0 and 14.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if (0 == os_strcmp(argv[i], "-d") && (i + 1) < argc)
            {
                db_file = argv[++i];
                run_cct = true;
                is_plain_mode = true;
            }
            else if (0 == os_strcmp(argv[i], "-r") && (i + 1) < argc)
            {
                report_file = argv[++i];
            }
            else if (0 == os_strcmp(argv[i], "-e") && (i + 1) < argc)
            {
                eds_file = argv[++i];
                run_cct = true;
                is_plain_mode = true;
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
            else if (0 == os_strcmp(argv[i], "-w") && (i + 1) < argc)
            {
                char* endptr;

                uint32 timeout_ms = (uint32)os_strtol(argv[++i], &endptr, 0);
                if (*endptr != '\0')
                {
                    os_printf("Invalid timeout value.\n");
                    exit(EXIT_FAILURE);
                }
                sdo_set_timeout(timeout_ms);
            }
            else if (0 == os_strcmp(argv[i], "-f"))
            {
                fullscreen = true;
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
                os_printf("    -s SCRIPT         Run script (.lua can be omitted)\n");
                os_printf("    -i INTERFACE      Set CAN interface\n");
                os_printf("    -b BAUD           Set baud rate\n");
                os_printf("                        1 = 1 MBit/s\n");
                os_printf("                        2 = 800 kBit/s\n");
                os_printf("                        3 = 500 kBit/s\n");
                os_printf("                        4 = 250 kBit/s\n");
                os_printf("                        5 = 125 kBit/s\n");
                os_printf("    -d DATABASE       Load custom database file, implies -t\n");
                os_printf("    -r REPORT         Generate conformance test report (JUnit XML)\n");
                os_printf("    -e EDS_FILE       Test device conformance, implies -t\n");
                os_printf("    -n NODE_ID        Set node ID, default: 0x01\n");
                os_printf("    -w TIMEOUT_MS     Set SDO timeout in milliseconds\n");
                os_printf("    -f                Full screen widget window\n");
                os_printf("    -p                Run in plain mode\n");
                os_printf("    -t                Run conformance test, implies -p\n");

                exit(EXIT_FAILURE);
            }
        }
    }

    if (core_init(&core, is_plain_mode) != ALL_OK)
    {
        os_log(LOG_ERROR, "Failed to initialize core.");
        return EXIT_FAILURE;
    }

    if (core == NULL)
    {
        return EXIT_FAILURE;
    }

    if (db_file != NULL)
    {
        if (load_codb_ex(db_file) != ALL_OK)
        {
            os_log(LOG_ERROR, "Failed to load database file: %s", db_file);
            return EXIT_FAILURE;
        }
    }

    if (true == run_cct)
    {
        core->is_test_mode = true;
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
    else if (true == fullscreen)
    {
        window_fullscreen(fullscreen);
    }

    if (true == run_cct)
    {
        /* Wait for DS301 database to be loaded before running conformance test. */
        while (false == is_ds301_loaded()) {}
        cct_run_test(node_id, report_file, eds_file);
        core->is_running = false;
    }
    else if (script != NULL)
    {
        run_script(script, core);
        core->is_running = false;
    }

    while (true == core->is_running)
    {
        core_update((void*)core);
        os_delay(10);
    }
    core_deinit(core);
    os_print(DEFAULT_COLOR, "\nKeep the Heartbeat alive.");
    os_print(DARK_RED, " \u2665 ");
    os_print(DEFAULT_COLOR, "Consider supporting CANopenTerm.\n");

    return status;
}
