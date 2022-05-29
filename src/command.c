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
        can_message_t can_message = { 0 };
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

        sdo_read(&can_message, SDL_TRUE, node_id, sdo_index, sub_index);
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
    color_t frame_color = DARK_CYAN;
    color_t text_color  = DARK_WHITE;

    c_printf(frame_color, " ┌─────╥───────────────────────────────╥──────────────┐\r\n │ ");
    c_printf(text_color,  "CMD ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "Parameter(s)                  ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "Function     ");
    c_printf(frame_color, "│\r\n ├─────╫───────────────────────────────╫──────────────┤\r\n │  ");
    c_printf(text_color,  "c  ");
    c_printf(frame_color, "║                               ║ ");
    c_printf(text_color,  "Clear output ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "g  ");
    c_printf(frame_color, "║                               ║ ");
    c_printf(text_color,  "Activate GUI ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "n  ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "[node_id] [command or alias]");
    c_printf(frame_color, "  ║ ");
    c_printf(text_color,  "NMT command  ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "l  ");
    c_printf(frame_color, "║                               ║ ");
    c_printf(text_color,  "List scripts ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "r  ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "[node_id] [index] (sub_index) ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "Read SDO     ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "s  ");
    c_printf(frame_color, "║ ");
    c_printf(text_color,  "[script_name]");
    c_printf(frame_color, "                 ║ ");
    c_printf(text_color,  "Run script   ");
    c_printf(frame_color, "│\r\n │  ");
    c_printf(text_color,  "q  ");
    c_printf(frame_color, "║                               ║ ");
    c_printf(text_color,  "Quit         ");
    c_printf(frame_color, "│\r\n └─────╨───────────────────────────────╨──────────────┘\r\n");
}
