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
#include "can.h"
#include "core.h"
#include "command.h"
#include "gui.h"
#include "nmt_client.h"
#include "printf.h"
#include "scripts.h"
#include "sdo_client.h"
#include "table.h"

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
            nmt_print_help();
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
        nmt_send_command((Uint16)node_id, (Uint8)command);
    }
    else if (0 == SDL_strncmp(token, "l", 1))
    {
        list_scripts();
    }
    else if (0 == SDL_strncmp(token, "r", 1))
    {
        can_message_t sdo_response = { 0 };
        Uint32        node_id;
        Uint32        sdo_index;
        Uint32        sub_index;

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
            convert_token_to_uint(token, &sdo_index);
        }

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            sub_index = 0;
        }
        else
        {
            convert_token_to_uint(token, &sub_index);
        }

        sdo_read(&sdo_response, SDL_TRUE, node_id, sdo_index, sub_index);
    }
    else if (0 == SDL_strncmp(token, "w", 1))
    {
        can_message_t sdo_response = { 0 };
        Uint32        node_id;
        Uint32        sdo_index;
        Uint32        sub_index;
        Uint32        sdo_data;
        Uint32        sdo_data_length;

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
            convert_token_to_uint(token, &sdo_index);
        }

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information();
            return;
        }
        else
        {
            convert_token_to_uint(token, &sub_index);
        }

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information();
            return;
        }
        else
        {
            convert_token_to_uint(token, &sdo_data_length);
        }

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            sdo_data = 0;
        }
        else
        {
            convert_token_to_uint(token, &sdo_data);
        }

        sdo_write(&sdo_response, SDL_TRUE, node_id, sdo_index, sub_index, sdo_data_length, sdo_data);
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
    table_t table = { DARK_CYAN, DARK_WHITE, 3, 45, 12 };

    table_print_header(&table);
    table_print_row("CMD", "Parameter(s)",                                  "Function",     &table);
    table_print_divider(&table);
    table_print_row(" c ", " ",                                             "Clear output", &table);
    table_print_row(" g ", " ",                                             "Activate GUI", &table);
    table_print_row(" l ", " ",                                             "List scripts", &table);
    table_print_row(" s ", "[script_name]",                                 "Run script",   &table);
    table_print_row(" n ", "[node_id] [command or alias]",                  "NMT command",  &table);
    table_print_row(" r ", "[node_id] [index] (sub_index)",                 "Read SDO",     &table);
    table_print_row(" w ", "[node_id] [index] [sub_index] [length] (data)", "Write SDO",    &table);
    table_print_row(" q ", " ",                                             "Quit",         &table);
    table_print_footer(&table);
}
