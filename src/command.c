/** @file command.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdio.h>
#include "SDL.h"
#include "core.h"
#include "command.h"
#include "gui.h"
#include "nmt_client.h"
#include "scripts.h"

#ifdef _WIN32
#  define CLEAR_CMD "cls"
#elif defined(unix) || defined(__unix__) || defined(__unix)
#  define CLEAR_CMD "clear"
#else
#  define CLEAR_CMD ""
#endif

static void convert_token_to_uint(char* token, Uint32* result);
static void print_usage_information(void);

void parse_command(char* input, core_t* core)
{
    int   index;
    char* delim        = " \n";
    char* context      = NULL;
    char* token        = NULL;
    char* input_savptr = input;

    token = SDL_strtokr(input_savptr, delim, &input_savptr);

    if (NULL == token)
    {
        print_usage_information();
        return;
    }
    else if (0 == SDL_strncmp(token, "c", 1))
    {
        system(CLEAR_CMD);
    }
    else if (0 == SDL_strncmp(token, "q", 1))
    {
        core->is_running = SDL_FALSE;
    }
    else if (0 == SDL_strncmp(token, "g", 1))
    {
        gui_init(core);
    }
    else if (0 == SDL_strncmp(token, "n", 1))
    {
        Uint32 node_id;
        Uint32 command;

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information();
            return;
        }
        else
        {
            convert_token_to_uint(token, &node_id);
        }

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information();
            return;
        }
        else
        {
            size_t token_len = SDL_strlen(token);

            if (0 == SDL_strncmp(token, "op", token_len))
            {
                command = NMT_OPERATIONAL;
            }
            else if (0 == SDL_strncmp(token, "stop", token_len))
            {
                command = NMT_STOP;
            }
            else if (0 == SDL_strncmp(token, "preop", token_len))
            {
                command = NMT_PRE_OPERATIONAL;
            }
            else if (0 == SDL_strncmp(token, "reset", token_len))
            {
                command = NMT_RESET_NODE;
            }
            else
            {
                convert_token_to_uint(token, &command);
            }
        }
        send_nmt_command((Uint16)node_id, (Uint8)command);
    }
    else if (0 == SDL_strncmp(token, "l", 1))
    {
        list_scripts();
    }
    else if (0 == SDL_strncmp(token, "s", 1))
    {
        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information();
            return;
        }
        else
        {
            run_script(token, core);
        }
    }
    else
    {
        print_usage_information();
    }
}

static void convert_token_to_uint(char* token, Uint32* result)
{
    if (('0' == token[0]) && ('x' == token[1]))
    {
        *result = (Uint32)SDL_strtol(token, NULL, 16);
    }
    else
    {
        *result = (Uint32)SDL_strtol(token, NULL, 10);
    }
}

static void print_usage_information(void)
{
    puts(" ┌─────╥──────────────────────────────╥──────────────┐");
    puts(" │ CMD ║ Parameter(s)                 ║ Function     │");
    puts(" ├─────╫──────────────────────────────╫──────────────┤");
    puts(" │  c  ║                              ║ Clear putput │");
    puts(" │  g  ║                              ║ Activate GUI │");
    puts(" │  n  ║ [node_id] [command or alias] ║ NMT command  │");
    puts(" │  l  ║                              ║ List scripts │");
    puts(" │  s  ║ [script_name]                ║ Run script   │");
    puts(" │  q  ║                              ║ Quit         │");
    puts(" └─────╨──────────────────────────────╨──────────────┘");
}
