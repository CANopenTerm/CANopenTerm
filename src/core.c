/** @file core.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "can.h"
#include "command.h"
#include "core.h"
#include "dict.h"
#include "nmt_client.h"
#include "pdo.h"
#include "printf.h"
#include "sdo_client.h"
#include "scripts.h"
#include "version.h"

status_t core_init(core_t **core)
{
    *core = (core_t*)SDL_calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        return COT_ERROR;
    }

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleTitle("CANopenTerm");
#endif

    c_printf(LIGHT_YELLOW,  "<");
    c_printf(LIGHT_GREEN,   ">");
    c_printf(DEFAULT_COLOR, " CANopenTerm v%u.%u.%u\r\n",
             VERSION_MAJOR,
             VERSION_MINOR,
             VERSION_BUILD);
    c_printf(DEFAULT_COLOR, "Copyright (c) 2024, Michael Fitzmayer.\r\n\r\n");

    /* Initialise SDL. */
    if (0 != SDL_InitSubSystem(SDL_INIT_TIMER))
    {
        c_log(LOG_ERROR, "Unable to initialise SDL timer sub-system: %s", SDL_GetError());
        return COT_ERROR;
    }

    /* Initialise Lua. */
    scripts_init((*core));
    if (NULL != (*core)->L)
    {
        lua_register_can_commands((*core));
        lua_register_dict_commands((*core));
        lua_register_nmt_command((*core));
        lua_register_pdo_commands((*core));
        lua_register_sdo_commands((*core));
    }

    /* Initialise CAN. */
    can_init((*core));

    (*core)->is_running = SDL_TRUE;
    return COT_OK;
}

status_t core_update(core_t* core)
{
#ifdef _WIN32
    char command[COMMAND_BUFFER_SIZE] = { 0 };
#elif defined __linux__
    char* command;
#endif

    if (NULL == core)
    {
        return COT_OK;
    }

#ifdef _WIN32
    c_print_prompt();
    if (NULL != fgets(command, COMMAND_BUFFER_SIZE - 1, stdin))
    {
        parse_command(command, core);
    }
    else
    {
        if (0 != feof(stdin))
        {
            core->is_running = SDL_FALSE;
        }
    }
#elif defined __linux__
    command = readline(": ");
    if (command != NULL)
    {
        add_history(command);
        parse_command(command, core);
    }
    free(command);
#endif

    return COT_OK;
}

void core_deinit(core_t *core)
{
    if (NULL == core)
    {
        return;
    }

    can_quit(core);
    scripts_deinit(core);
    SDL_Quit();
    SDL_free(core);
}
