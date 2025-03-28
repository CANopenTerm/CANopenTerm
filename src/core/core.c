/** @file core.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "can.h"
#include "codb.h"
#include "command.h"
#include "dbc.h"
#include "lua_can.h"
#include "lua_dbc.h"
#include "lua_misc.h"
#include "lua_nmt.h"
#include "lua_pdo.h"
#include "lua_sdo.h"
#include "lua_test_report.h"
#include "lua_widget.h"
#include "nmt.h"
#include "os.h"
#include "python_can.h"
#include "python_dbc.h"
#include "python_misc.h"
#include "python_nmt.h"
#include "python_pdo.h"
#include "python_sdo.h"
#include "python_test_report.h"
#include "python_widget.h"
#include "scripts.h"
#include "test_report.h"
#include "version.h"

status_t core_init(core_t** core, bool is_plain_mode)
{
    status_t status;

    *core = (core_t*)os_calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        return OS_MEMORY_ALLOCATION_ERROR;
    }

    status = os_console_init(is_plain_mode);
    if (status != ALL_OK)
    {
        return status;
    }

    (*core)->is_plain_mode = is_plain_mode;
    if (false == is_plain_mode)
    {
        os_print(LIGHT_YELLOW, "<");
        os_print(LIGHT_GREEN, ">");
        os_print(DEFAULT_COLOR, " CANopenTerm %u.%u.%u\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
        os_print(DEFAULT_COLOR, "Copyright (c) 2025, Michael Fitzmayer.\r\n\r\n");
    }

    status = os_init();
    if (status != ALL_OK)
    {
        return status;
    }

    scripts_init((*core));
    if (NULL != (*core)->L)
    {
        lua_register_can_commands((*core));
        lua_register_dbc_commands((*core));
        lua_register_misc_commands((*core));
        lua_register_nmt_command((*core));
        lua_register_pdo_commands((*core));
        lua_register_sdo_commands((*core));
        lua_register_test_commands((*core));
        lua_register_widget_commands((*core));
        python_can_init();
        python_dbc_init();
        python_misc_init();
        python_nmt_init();
        python_pdo_init();
        python_sdo_init();
        python_test_init();
        python_widget_init();
    }
    else
    {
        return SCRIPT_INIT_ERROR;
    }

    /* Initialise base database. */
    codb_init();

    /* Initialise CAN. */
    (*core)->is_running = true;
    can_init((*core)); /* Must becalled AFTER is_running has been set to true! */

    return status;
}

int core_update(void* core_pt)
{
    char command[COMMAND_BUFFER_SIZE] = {0};
    core_t* core = (core_t*)core_pt;

    if (NULL == core)
    {
        return 1;
    }

    while (true == core->is_running)
    {
        if (false == core->is_script_running)
        {
            os_print_prompt();
            if (ALL_OK == os_get_prompt(command))
            {
                parse_command(command, core);
            }
        }
    }

    return 0;
}

void core_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    test_clear_results();
    can_quit(core);
    codb_deinit();
    dbc_unload();
    scripts_deinit(core);
    os_quit();
    os_free(core);
}
