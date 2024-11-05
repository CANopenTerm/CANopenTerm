/** @file core.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "command.h"
#include "core.h"
#include "dbc.h"
#include "lua_can.h"
#include "lua_dbc.h"
#include "lua_misc.h"
#include "lua_nmt.h"
#include "lua_pdo.h"
#include "lua_sdo.h"
#include "nmt.h"
#include "os.h"
#include "scripts.h"
#include "version.h"

status_t core_init(core_t **core, bool_t is_silent)
{
    status_t status;

    *core = (core_t*)os_calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        return OS_MEMORY_ALLOCATION_ERROR;
    }

    status = os_console_init();
    if (status != ALL_OK)
    {
        return status;
    }

    if (IS_FALSE == is_silent)
    {
        os_print(LIGHT_YELLOW, "<");
        os_print(LIGHT_GREEN, ">");
        os_print(DEFAULT_COLOR, " CANopenTerm %u.%u.%u\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
        os_print(DEFAULT_COLOR, "Copyright (c) 2024, Michael Fitzmayer.\r\n\r\n");
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
    }
    else
    {
        return SCRIPT_INIT_ERROR;
    }

    /* Initialise CAN. */
    can_init((*core));

    (*core)->is_running = IS_TRUE;
    return status;
}

status_t core_update(core_t* core)
{
    char command[COMMAND_BUFFER_SIZE] = { 0 };

    if (NULL == core)
    {
        return ALL_OK;
    }

    os_print_prompt();
    if (ALL_OK == os_get_prompt(command))
    {
        parse_command(command, core);
    }

    return ALL_OK;
}

void core_deinit(core_t *core)
{
    if (NULL == core)
    {
        return;
    }

    dbc_unload();
    can_quit(core);
    scripts_deinit(core);
    os_quit();
    os_free(core);
}
