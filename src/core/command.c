/** @file command.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>

#include "can.h"
#include "command.h"
#include "core.h"
#include "dict.h"
#include "eds.h"
#include "nmt.h"
#include "os.h"
#include "pdo.h"
#include "scripts.h"
#include "sdo.h"
#include "table.h"

static void convert_token_to_uint(char* token, uint32* result);
static void convert_token_to_uint64(char* token, uint64* result);
status_t print_usage_information(bool show_all);
static bool is_numeric(const char* str);

void parse_command(char* input, core_t* core)
{
    int index;
    char* delim = " \n";
    char* context;
    char* token = NULL;
    char* input_savptr = input;
    uint32 command;

    token = os_strtokr_r(input_savptr, delim, &input_savptr);

    if (NULL == token)
    {
        return;
    }
    else if ((input[1] != ' ') && (input[1] != '\0'))
    {
        core->user_event.type = SDL_EVENT_USER;
        core->user_event.user.code = RUN_SCRIPT_EVENT;
        core->user_event.user.data1 = token;
        core->user_event.user.data2 = NULL;
        SDL_PushEvent(&core->user_event);
        return;
    }
    else if (0 == os_strncmp(token, "y", 1))
    {
        uint32 channel_index;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
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
    else if (0 == os_strncmp(token, "b", 1))
    {
        uint32 baud_rate_index;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
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
        uint32 sub_index;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            list_codb();
            return;
        }

        convert_token_to_uint(token, &file_no);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            load_codb(file_no);
        }
        else
        {
            convert_token_to_uint(token, &sub_index);
            (void)dict_lookup_object(file_no, sub_index);
        }
    }
    else if (0 == os_strncmp(token, "q", 1))
    {
        core->is_running = false;
    }
    else if (0 == os_strncmp(token, "h", 1))
    {
        print_usage_information(true);
    }
    else if (0 == os_strncmp(token, "n", 1))
    {
        uint32 node_id;
        uint32 command;
        size_t token_len;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(false);
            return;
        }

        convert_token_to_uint(token, &node_id);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
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

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(false);
            return;
        }
        else if (0 == os_strncmp(token, "add", 3))
        {
            uint32 event_time_ms;
            uint32 length;
            uint64 data;

            token = os_strtokr_r(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(false);
                return;
            }

            convert_token_to_uint(token, &can_id);

            token = os_strtokr_r(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(false);
                return;
            }

            convert_token_to_uint(token, &event_time_ms);

            token = os_strtokr_r(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(false);
                return;
            }

            convert_token_to_uint(token, &length);

            token = os_strtokr_r(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(false);
                return;
            }

            convert_token_to_uint64(token, &data);

            if (false == pdo_is_id_valid(can_id))
            {
                pdo_print_help();
                return;
            }

            if (false == is_can_initialised(core))
            {
                os_log(LOG_WARNING, "Could not add PDO: CAN not initialised");
                return;
            }

            pdo_add((uint16)can_id, event_time_ms, length, data, TERM_MODE);
        }
        else if (0 == os_strncmp(token, "del", 3))
        {
            token = os_strtokr_r(input_savptr, delim, &input_savptr);
            if (NULL == token)
            {
                print_usage_information(false);
                return;
            }

            convert_token_to_uint(token, &can_id);

            if (false == pdo_is_id_valid(can_id))
            {
                pdo_print_help();
                return;
            }

            if (false == is_can_initialised(core))
            {
                os_log(LOG_WARNING, "Could not delete PDO: CAN not initialised");
                return;
            }

            pdo_del((uint16)can_id, TERM_MODE);
        }
        else
        {
            print_usage_information(false);
            return;
        }
    }
    else if (0 == os_strncmp(token, "r", 1))
    {
        can_message_t sdo_response = {0};
        uint32 node_id;
        uint32 sdo_index;
        uint32 sub_index = 0;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(false);
            return;
        }

        convert_token_to_uint(token, &node_id);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(false);
            return;
        }

        convert_token_to_uint(token, &sdo_index);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL != token)
        {
            convert_token_to_uint(token, &sub_index);
        }

        sdo_read(&sdo_response, TERM_MODE, node_id, sdo_index, sub_index, NULL);
    }
    else if (0 == os_strncmp(token, "w", 1))
    {
        can_message_t sdo_response = {0};
        uint32 node_id;
        uint32 sdo_index;
        uint32 sub_index;
        uint32 sdo_data_length = 0;
        uint32 sdo_data = 0;
        sdo_state_t sdo_state = IS_WRITE_EXPEDITED;

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(false);
            return;
        }
        convert_token_to_uint(token, &node_id);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(false);
            return;
        }
        convert_token_to_uint(token, &sdo_index);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (token == NULL)
        {
            print_usage_information(false);
            return;
        }
        convert_token_to_uint(token, &sub_index);

        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (token != NULL)
        {
            char buffer[256] = {0};

            if (is_numeric(token))
            {
                convert_token_to_uint(token, &sdo_data_length);

                token = os_strtokr_r(input_savptr, delim, &input_savptr);
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
                token = os_strtokr_r(NULL, delim, &input_savptr);

                while (token != NULL)
                {
                    os_strlcat(buffer, " ", sizeof(buffer));
                    os_strlcat(buffer, token, sizeof(buffer));
                    len = os_strlen(buffer);
                    token = os_strtokr_r(NULL, delim, &input_savptr);
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
                print_usage_information(false);
                return;
            }
        }
        else
        {
            print_usage_information(false);
            return;
        }
    }
    else if (0 == os_strncmp(token, "s", 1))
    {
        token = os_strtokr_r(input_savptr, delim, &input_savptr);
        if (NULL == token)
        {
            print_usage_information(true);
            return;
        }

        core->user_event.type = SDL_EVENT_USER;
        core->user_event.user.code = RUN_SCRIPT_EVENT;
        core->user_event.user.data1 = token;
        core->user_event.user.data2 = NULL;
        SDL_PushEvent(&core->user_event);
    }
    else
    {
        print_usage_information(false);
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

status_t print_usage_information(bool show_all)
{
    status_t status;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 45, 17};

    status = table_init(&table, 1024);
    table_print_header(&table);
    table_print_row("CMD", "Parameter(s)", "Function", &table);
    table_print_divider(&table);
    table_print_row(" h ", " ", "Show full help", &table);

    if (true == show_all)
    {
        table_print_row(" b ", "(identifer)", "Set baud rate", &table);
        table_print_row(" d ", "[file_no]", "Load data base", &table);
        table_print_row(" d ", "[index] [sub_index]", "Lookup dictionary", &table);
        table_print_row(" y ", "(identifer)", "Set CAN channel", &table);
        table_print_row(" c ", " ", "Clear output", &table);
        table_print_row(" l ", " ", "List scripts", &table);
        table_print_row("(s)", "[identifier](.lua)", "Run script", &table);
    }

    table_print_row(" n ", "[node_id] [command or alias]", "NMT command", &table);
    table_print_row(" r ", "[node_id] [index] (sub_index)", "Read SDO", &table);
    table_print_row(" w ", "[node_id] [index] [sub_index] [length] (data)", "Write SDO", &table);
    table_print_row(" w ", "[node_id] [index] [sub_index] [\"data\"]", "Write SDO", &table);
    table_print_row(" p ", "add [can_id] [event_time_ms] [length] [data]", "Add PDO (tx)", &table);
    table_print_row(" p ", "del [can_id]", "Remove PDO (tx)", &table);
    table_print_row(" q ", " ", "Quit", &table);
    table_print_footer(&table);
    table_flush(&table);

    return status;
}

static bool is_numeric(const char* str)
{
    if ((NULL == str) || ('\0' == *str))
    {
        return false;
    }

    while (*str)
    {
        if (0 == os_isdigit(*str))
        {
            return false;
        }
        str += 1;
    }

    return true;
}

void completion_callback(const char* buf, completions_t* lc)
{
    if (buf[0] == '\0')
    {
        // Empty line TAB -> suggest commands.
        os_completion_add(lc, "h", "Show full help");
        os_completion_add(lc, "b", "Set baud rate");
        os_completion_add(lc, "d", "Load data base");
        os_completion_add(lc, "d", "Lookup dictionary");
        os_completion_add(lc, "y", "Set CAN channel");
        os_completion_add(lc, "c", "Clear output");
        os_completion_add(lc, "l", "List scripts");
        os_completion_add(lc, "s", "Run script");
        os_completion_add(lc, "n", "NMT command");
        os_completion_add(lc, "r", "Read SDO");
        os_completion_add(lc, "w", "Write SDO");
        os_completion_add(lc, "p", "PDO");
        os_completion_add(lc, "q", "Quit");
    }
    else if (buf[0] == 'b' && buf[2] == '\0')
    {
        os_completion_add(lc, "b 0", "Set 1 MBit/s");
        os_completion_add(lc, "b 1", "Set 800 kBit/s");
        os_completion_add(lc, "b 2", "Set 500 kBit/s");
        os_completion_add(lc, "b 3", "Set 250 kBit/s");
        os_completion_add(lc, "b 4", "Set 125 kBit/s");
        os_completion_add(lc, "b 5", "Set 100 kBit/s");
        os_completion_add(lc, "b 6", "Set 95.238 kBit/s");
        os_completion_add(lc, "b 7", "Set 83.333 kBit/s");
        os_completion_add(lc, "b 8", "Set 50 kBit/s");
        os_completion_add(lc, "b 9", "Set 47.619 kBit/s");
        os_completion_add(lc, "b 10", "Set 33.333 kBit/s");
        os_completion_add(lc, "b 11", "Set 20 kBit/s");
        os_completion_add(lc, "b 12", "Set 10 kBit/s");
        os_completion_add(lc, "b 13", "Set 5 kBit/s");
    }
    else if (buf[0] == 'b' && buf[2] == '1' && buf[3] == '\0')
    {
        os_completion_add(lc, "b 1", "Set 800 kBit/s");
        os_completion_add(lc, "b 10", "Set 33.333 kBit/s");
        os_completion_add(lc, "b 11", "Set 20 kBit/s");
        os_completion_add(lc, "b 12", "Set 10 kBit/s");
        os_completion_add(lc, "b 13", "Set 5 kBit/s");
    }
    else if (buf[0] == 'd' && buf[2] == '\0')
    {
        os_completion_add(lc, "d 2", "[CiA 302] Programmable CANopen Devices");
        os_completion_add(lc, "d 3", "[CiA 401] I/O devices");
        os_completion_add(lc, "d 4", "[CiA 402] Drives and motion control");
        os_completion_add(lc, "d 5", "[CiA 404] Measurement Devices and Closed Loop Controllers");
        os_completion_add(lc, "d 6", "[CiA 405] IEC 61131-3 programmable devices");
        os_completion_add(lc, "d 7", "[CiA 406] Encoders");
        os_completion_add(lc, "d 8", "[CiA 408] Fluid Power Technology");
        os_completion_add(lc, "d 9", "[CiA 410] Inclinometer");
        os_completion_add(lc, "d 10", "[CiA 413] Truck Gateways");
        os_completion_add(lc, "d 11", "[CiA 415] Road construction machinery");
        os_completion_add(lc, "d 12", "[CiA 416] Building door control");
        os_completion_add(lc, "d 13", "[CiA 417] Lift control systems");
        os_completion_add(lc, "d 14", "[CiA 418] Battery modules");
        os_completion_add(lc, "d 15", "[CiA 419] Battery chargers");
        os_completion_add(lc, "d 16", "[CiA 443] SIIS level-2 devices");
        os_completion_add(lc, "d 17", "[CiA 447] Special-purpose car add-on devices");
    }
    else if (buf[0] == 'd' && buf[2] == '1' && buf[3] == '\0')
    {
        os_completion_add(lc, "d 10", "[CiA 413] Truck Gateways");
        os_completion_add(lc, "d 11", "[CiA 415] Road construction machinery");
        os_completion_add(lc, "d 12", "[CiA 416] Building door control");
        os_completion_add(lc, "d 13", "[CiA 417] Lift control systems");
        os_completion_add(lc, "d 14", "[CiA 418] Battery modules");
        os_completion_add(lc, "d 15", "[CiA 419] Battery chargers");
        os_completion_add(lc, "d 16", "[CiA 443] SIIS level-2 devices");
        os_completion_add(lc, "d 17", "[CiA 447] Special-purpose car add-on devices");
    }
    else if (buf[0] == 'p' && buf[2] == '\0')
    {
        os_completion_add(lc, "p add", "Add PDO (tx)");
        os_completion_add(lc, "p del", "Remove PDO (tx)");
    }
    else if (buf[0] == 'n')
    {
        unsigned val;
        char dummy;
        if (os_sscanf(buf, "n %i %c", &val, &dummy) == 1)
        {
            char tmp[256] = {0};
            os_snprintf(tmp, 256, "n 0x%x op", val);
            os_completion_add(lc, tmp, "Start (go to Operational)");
            os_snprintf(tmp, 256, "n 0x%x stop", val);
            os_completion_add(lc, tmp, "Stop (go to Stopped)");
            os_snprintf(tmp, 256, "n 0x%x preop", val);
            os_completion_add(lc, tmp, "Go to Pre-operational");
            os_snprintf(tmp, 256, "n 0x%x reset", val);
            os_completion_add(lc, tmp, "Reset node (Application reset)");
            os_snprintf(tmp, 256, "n 0x%x 0x82", val);
            os_completion_add(lc, tmp, "Reset communication");
        }
    }
    else if (buf[0] == 'r')
    {
        unsigned val;
        char dummy;
        if (os_sscanf(buf, "r %i %c", &val, &dummy) == 1)
        {
            char tmp[256] = {0};
            os_snprintf(tmp, 256, "r 0x%x 0x1000", val);
            os_completion_add(lc, tmp, "Read Device Type");
            os_snprintf(tmp, 256, "r 0x%x 0x1008", val);
            os_completion_add(lc, tmp, "Read Manufacturer Device Name");
            os_snprintf(tmp, 256, "r 0x%x 0x1009", val);
            os_completion_add(lc, tmp, "Read Manufacturer Hardware Version");
            os_snprintf(tmp, 256, "r 0x%x 0x100a", val);
            os_completion_add(lc, tmp, "Read Manufacturer Software Version");
        }
    }
}
