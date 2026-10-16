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
#include "command.h"
#include "core.h"
#include "ctt.h"
#include "os.h"
#include "scripts.h"
#include "sdo.h"
#include "server.h"
#include "tachometer.h"
#include "window.h"

core_t* core = NULL;

int main(int argc, char* argv[])
{
    bool is_plain_mode = false;
    bool run_cct = false;
    bool fullscreen = false;
    bool is_headless = false;
    bool has_server_option = false;
    bool allow_server_scripts = false;
    char* host = SERVER_DEFAULT_HOST;
    uint16 port = SERVER_DEFAULT_PORT;
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
            else if (0 == os_strcmp(argv[i], "-S"))
            {
                is_headless = true;
                is_plain_mode = true;
            }
            else if (0 == os_strcmp(argv[i], "-A"))
            {
                allow_server_scripts = true;
                has_server_option = true;
            }
            else if (0 == os_strcmp(argv[i], "-a") && (i + 1) < argc)
            {
                host = argv[++i];
                has_server_option = true;
            }
            else if (0 == os_strcmp(argv[i], "-P") && (i + 1) < argc)
            {
                char* endptr;
                long value = os_strtol(argv[++i], &endptr, 0);

                if (value < 1 || value > 65535 || *endptr != '\0')
                {
                    os_printf("Invalid port.  Must be between 1 and 65535.\n");
                    exit(EXIT_FAILURE);
                }
                port = (uint16)value;
                has_server_option = true;
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
                os_printf("    -h                Show this help\n");
                os_printf("    -S                Run headless as WebSocket server, implies -p\n");
                os_printf("                      Ctrl+C quits unless a client is connected\n");
                os_printf("    -a ADDRESS        Server bind address, default: %s\n", SERVER_DEFAULT_HOST);
                os_printf("    -P PORT           Server port, default: %d\n", SERVER_DEFAULT_PORT);
                os_printf("    -A                Allow scripts in server mode\n");

                exit((0 == os_strcmp(argv[i], "-h")) ? EXIT_SUCCESS : EXIT_FAILURE);
            }
        }
    }

    if (true == has_server_option && false == is_headless)
    {
        os_printf("Options -a, -A and -P have no effect without -S.\n");
    }

    if (core_init(&core, is_plain_mode, is_headless, allow_server_scripts) != ALL_OK)
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

    if (true == is_headless)
    {
        /* No window in server mode. */
    }
    else if (! window_init(core))
    {
        /* Nothing to do here. */
    }
    else if (true == fullscreen)
    {
        window_fullscreen(fullscreen);
    }

    if (true == is_headless)
    {
        if (server_init(host, port) != ALL_OK)
        {
            core_deinit(core);
            return EXIT_FAILURE;
        }
    }

    if ((true == is_headless) && (true == run_cct))
    {
        os_log(LOG_WARNING, "Tests are not available in server mode.");
    }
    else if ((true == is_headless) && (script != NULL) && (false == allow_server_scripts))
    {
        os_log(LOG_WARNING, "Scripts are not available in server mode.  Use -A to allow them.");
    }
    else if (true == run_cct)
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
        if (true == is_headless)
        {
            char command[COMMAND_BUFFER_SIZE] = {0};

            if (true == server_is_quit_requested())
            {
                core->is_running = false;
                break;
            }

            if (true == server_update(command))
            {
                if (command[0] != '\0')
                {
                    parse_command(command, core, TERM_MODE);
                }
                os_print_prompt();
            }
        }
        else
        {
            core_update((void*)core);
        }
        os_delay(10);
    }
    server_deinit();
    core_deinit(core);
    os_print(DEFAULT_COLOR, "\nKeep the Heartbeat alive.");
    os_print(DARK_RED, " \u2665 ");
    os_print(DEFAULT_COLOR, "Consider supporting CANopenTerm.\n");

    return status;
}
