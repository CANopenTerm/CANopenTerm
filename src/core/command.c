/** @file command.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "command.h"
#include "eds.h"
#include "nmt.h"
#include "os.h"
#include "pdo.h"
#include "scripts.h"
#include "sdo.h"
#include "table.h"

static void   convert_token_to_uint(char* token, uint32* result);
static void   convert_token_to_uint64(char* token, uint64* result);
status_t      print_usage_information(bool_t show_all);
static bool_t is_numeric(const char* str);

void parse_command(char* input, core_t* core)
{
    int    index;
    char*  delim        = " \n";
    char*  context;
    char*  token        = NULL;
    char*  input_savptr = input;
    uint32 command;

    token = os_strtokr(input_savptr, delim, &input_savptr);

    if (NULL == token)
    {
        return;
    }
    else if ((input[1] != ' ') && (input[1] != '\0'))
    {
        run_script(token, core);
        return;
    }
    else if (0 == SDL_strncmp(token, "y", 1))
    {
        uint32 channel_index;

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            can_print_channel_help(core);
            return;
        }
        else
        {
            convert_token_to_uint(token, &channel_index);
        }

        can_set_channel(channel_index, core);
    }
    else if (0 == SDL_strncmp(token, "b", 1))
    {
        uint32 baud_rate_index;

        token = SDL_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            can_print_baud_rate_help(core);
            return;
        }
        else
        {
            convert_token_to_uint(token, &baud_rate_index);
        }

        can_set_baud_rate(baud_rate_index, core);
    }
    else if (0 == os_strncmp(token, "c", 1))
    {
        if (0 != system(CLEAR_CMD))
        {
            os_log(LOG_WARNING, "Could not clear screen");
        }
    }
    else if (0 == os_strncmp(token, "d", 1))
    {
        uint32 file_no;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            list_codb();
            return;
        }

        convert_token_to_uint(token, &file_no);

        load_codb(file_no);
    }
    else if (0 == os_strncmp(token, "q", 1))
    {
        core->is_running = IS_FALSE;
    }
    else if (0 == os_strncmp(token, "h", 1))
    {
        print_usage_information(IS_TRUE);
    }
    else if (0 == os_strncmp(token, "n", 1))
    {
        uint32 node_id;
        uint32 command;
        size_t token_len;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_FALSE);
            return;
        }

        convert_token_to_uint(token, &node_id);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            nmt_print_help(TERM_MODE);
            return;
        }

        token_len = os_strlen(token);

        if (0 == os_strncmp(token, "op", token_len))
        {
            command = NMT_OPERATIONAL;
        }
        else if (0 == os_strncmp(token, "stop", token_len))
        {
            command = NMT_STOP;
        }
        else if (0 == os_strncmp(token, "preop", token_len))
        {
            command = NMT_PRE_OPERATIONAL;
        }
        else if (0 == os_strncmp(token, "reset", token_len))
        {
            command = NMT_RESET_NODE;
        }
        else
        {
            convert_token_to_uint(token, &command);
        }

        nmt_send_command((uint16)node_id, (uint8)command, TERM_MODE, NULL);
    }
    else if (0 == os_strncmp(token, "l", 1))
    {
        list_scripts();
    }
    else if (0 == os_strncmp(token, "p", 1))
    {
        uint32 can_id;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_FALSE);
            return;
        }
        else if (0 == os_strncmp(token, "add", 3))
        {
            uint32 event_time_ms;
            uint32 length;
            uint64 data;

            token = os_strtokr(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(IS_FALSE);
                return;
            }

            convert_token_to_uint(token, &can_id);

            token = os_strtokr(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(IS_FALSE);
                return;
            }

            convert_token_to_uint(token, &event_time_ms);

            token = os_strtokr(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(IS_FALSE);
                return;
            }

            convert_token_to_uint(token, &length);

            token = os_strtokr(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(IS_FALSE);
                return;
            }

            convert_token_to_uint64(token, &data);

            if (IS_FALSE == pdo_is_id_valid(can_id))
            {
                pdo_print_help();
                return;
            }

            if (IS_FALSE == is_can_initialised(core))
            {
                os_log(LOG_WARNING, "Could not add PDO: CAN not initialised");
                return;
            }

            pdo_add((uint16)can_id, event_time_ms, length, data, TERM_MODE);
        }
        else if (0 == os_strncmp(token, "del", 3))
        {
            token = os_strtokr(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(IS_FALSE);
                return;
            }

            convert_token_to_uint(token, &can_id);

            if (IS_FALSE == pdo_is_id_valid(can_id))
            {
                pdo_print_help();
                return;
            }

            if (IS_FALSE == is_can_initialised(core))
            {
                os_log(LOG_WARNING, "Could not delete PDO: CAN not initialised");
                return;
            }

            pdo_del((uint16)can_id, TERM_MODE);
        }
        else
        {
            print_usage_information(IS_FALSE);
            return;
        }
    }
    else if (0 == os_strncmp(token, "r", 1))
    {
        can_message_t sdo_response = { 0 };
        uint32        node_id;
        uint32        sdo_index;
        uint32        sub_index    = 0;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_FALSE);
            return;
        }

        convert_token_to_uint(token, &node_id);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_FALSE);
            return;
        }

        convert_token_to_uint(token, &sdo_index);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL != token)
        {
            convert_token_to_uint(token, &sub_index);
        }

        sdo_read(&sdo_response, TERM_MODE, node_id, sdo_index, sub_index, NULL);
    }
    else if (0 == os_strncmp(token, "w", 1))
    {
        can_message_t sdo_response    = { 0 };
        uint32        node_id;
        uint32        sdo_index;
        uint32        sub_index;
        uint32        sdo_data_length = 0;
        uint32        sdo_data        = 0;
        sdo_state_t   sdo_state       = IS_WRITE_EXPEDITED;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(IS_FALSE);
            return;
        }
        convert_token_to_uint(token, &node_id);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(IS_FALSE);
            return;
        }
        convert_token_to_uint(token, &sdo_index);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(IS_FALSE);
            return;
        }
        convert_token_to_uint(token, &sub_index);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (token != NULL)
        {
            char buffer[256] = { 0 };

            if (is_numeric(token))
            {
                convert_token_to_uint(token, &sdo_data_length);

                token = os_strtokr(input_savptr, delim, &input_savptr);
                if (token != NULL)
                {
                    convert_token_to_uint(token, &sdo_data);
                }

                os_memcpy(buffer, &sdo_data, sizeof(uint32));
            }
            else
            {
                size_t len;

                os_strlcpy(buffer, token, sizeof(buffer));
                len = os_strlen(buffer);
                sdo_state = IS_WRITE_SEGMENTED;
                token = os_strtokr(NULL, delim, &input_savptr);

                while (token != NULL)
                {
                    os_strlcat(buffer, " ",   sizeof(buffer));
                    os_strlcat(buffer, token, sizeof(buffer));
                    len = os_strlen(buffer);
                    token = os_strtokr(NULL, delim, &input_savptr);
                }

                if (buffer[0] == '"' && buffer[len - 1] == '"')
                {
                    buffer[len - 1] = '\0';
                    os_memmove(buffer, buffer + 1, len - 1);
                }

                sdo_data_length = os_strlen(buffer);
            }

            if (sdo_data_length > 0)
            {
                if (IS_WRITE_EXPEDITED == sdo_state)
                {
                    sdo_write(&sdo_response, TERM_MODE, node_id, sdo_index, sub_index, sdo_data_length, (void*)buffer, NULL);
                }
                else
                {
                    sdo_write_segmented(&sdo_response, TERM_MODE, node_id, sdo_index, sub_index, sdo_data_length, (void*)buffer, NULL);
                }
            }
            else
            {
                print_usage_information(IS_FALSE);
                return;
            }
        }
        else
        {
            print_usage_information(IS_FALSE);
            return;
        }
    }
    else if (0 == os_strncmp(token, "s", 1))
    {
        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_TRUE);
            return;
        }
        run_script(token, core);
    }
    else if (0 == os_strncmp(token, "t", 1))
    {
        uint32 node_id;
        uint32 file_no;

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(IS_TRUE);
            return;
        }
        convert_token_to_uint(token, &node_id);

        token = os_strtokr(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            list_eds();
            return;
        }
        convert_token_to_uint(token, &file_no);

        validate_eds(file_no, node_id);
    }
    else
    {
        print_usage_information(IS_FALSE);
    }
}

static void convert_token_to_uint(char* token, uint32* result)
{
    if (('0' == token[0]) && ('x' == token[1]))
    {
        *result = (uint32)os_strtol(token, NULL, 16);
    }
    else
    {
        *result = (uint32)os_strtol(token, NULL, 10);
    }
}

static void convert_token_to_uint64(char* token, uint64* result)
{
    if (('0' == token[0]) && ('x' == token[1]))
    {
        *result = os_strtoull(token, NULL, 16);
    }
    else
    {
        *result = os_strtoull(token, NULL, 10);
    }
}

status_t print_usage_information(bool_t show_all)
{
    status_t status;
    table_t  table = { DARK_CYAN, DARK_WHITE, 3, 45, 16 };

    status = table_init(&table, 1024);
    table_print_header(&table);
    table_print_row("CMD", "Parameter(s)",                                  "Function",     &table);
    table_print_divider(&table);
    table_print_row(" h ", " ",                                             "Show full help", &table);

    if (IS_TRUE == show_all)
    {
        table_print_row(" b ", "(identifer)",                               "Set baud rate",    &table);
        table_print_row(" d ", "[file_no]",                                 "Load data base",   &table);
        table_print_row(" y ", "(identifer)",                               "Set CAN channel",  &table);
        table_print_row(" c ", " ",                                         "Clear output",     &table);
        table_print_row(" l ", " ",                                         "List scripts",     &table);
        table_print_row("(s)", "[identifier](.lua)",                        "Run script",       &table);
        table_print_row(" t ", "[node_id] [file no.]",                      "Conformance test", &table);
    }

    table_print_row(" n ", "[node_id] [command or alias]",                  "NMT command", &table);
    table_print_row(" r ", "[node_id] [index] (sub_index)",                 "Read SDO",    &table);
    table_print_row(" w ", "[node_id] [index] [sub_index] [length] (data)", "Write SDO",   &table);
    table_print_row(" w ", "[node_id] [index] [sub_index] [\"data\"]",      "Write SDO",   &table);
    table_print_row(" p ", "add [can_id] [event_time_ms] [length] [data]",  "Add TPDO",    &table);
    table_print_row(" p ", "del [can_id]",                                  "Remove TPDO", &table);
    table_print_row(" q ", " ",                                             "Quit",        &table);
    table_print_footer(&table);
    table_flush(&table);

    return status;
}

static bool_t is_numeric(const char* str)
{
    if ((NULL == str) || ('\0' == *str))
    {
        return IS_FALSE;
    }

    while (*str)
    {
        if (0 == os_isdigit(*str))
        {
            return IS_FALSE;
        }
        str += 1;
    }

    return IS_TRUE;
}
