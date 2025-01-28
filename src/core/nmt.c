/** @file nmt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "nmt.h"
#include "can.h"
#include "core.h"
#include "table.h"

void nmt_print_error(const char* reason, nmt_command_t command, disp_mode_t disp_mode);

status_t nmt_send_command(uint8 node_id, nmt_command_t command, disp_mode_t disp_mode, const char* comment)
{
    status_t      status      = ALL_OK;
    uint32        can_status  = 0;
    can_message_t can_message = {0};

    limit_node_id(&node_id);

    can_message.id      = 0x000;
    can_message.length  = 2;
    can_message.data[0] = command;
    can_message.data[1] = node_id;

    switch (command)
    {
        case NMT_OPERATIONAL:
        case NMT_STOP:
        case NMT_PRE_OPERATIONAL:
        case NMT_RESET_NODE:
        case NMT_RESET_COMM:
            break;
        default:
            nmt_print_help(disp_mode);
            return NMT_UNKNOWN_COMMAND;
    }

    can_status = can_write(&can_message, SILENT, NULL);
    if (0 != can_status)
    {
        nmt_print_error(can_get_error_message(can_status), command, disp_mode);
        status = CAN_WRITE_ERROR;
    }
    else
    {
        if (SCRIPT_MODE == disp_mode)
        {
            int  i;
            char buffer[34] = {0};

            os_print(DARK_CYAN, "NMT  ");
            os_print(DEFAULT_COLOR, "    0x%02X    -       -         -       ", node_id);
            os_print(LIGHT_GREEN, "SUCC    ");

            if (NULL == comment)
            {
                comment = "-";
            }

            os_strlcpy(buffer, comment, 33);
            for (i = os_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            os_print(DARK_MAGENTA, "%s ", buffer);

            switch (command)
            {
                case 0x01:
                    os_print(DEFAULT_COLOR, "Start (go to Operational)\n");
                    break;
                case 0x02:
                    os_print(DEFAULT_COLOR, "Stop (go to Stopped)\n");
                    break;
                case 0x80:
                    os_print(DEFAULT_COLOR, "Go to Pre-operational\n");
                    break;
                case 0x81:
                    os_print(DEFAULT_COLOR, "Reset node (Application reset)\n");
                    break;
                case 0x82:
                    os_print(DEFAULT_COLOR, "Reset communication\n");
                    break;
            }
        }
    }

    return status;
}

status_t nmt_print_help(disp_mode_t disp_mode)
{
    status_t status;

    if (SILENT == disp_mode)
    {
        status = NOTHING_TO_DO;
    }
    else
    {
        table_t table = {DARK_CYAN, DARK_WHITE, 4, 5, 30};

        status = table_init(&table, 1024);
        if (ALL_OK == status)
        {
            table_print_header(&table);
            table_print_row("CMD", "Alias", "Description", &table);
            table_print_divider(&table);
            table_print_row("0x01", "op", "Start (go to Operational)", &table);
            table_print_row("0x02", "stop", "Stop (go to Stopped)", &table);
            table_print_row("0x80", "preop", "Go to Pre-operational", &table);
            table_print_row("0x81", "reset", "Reset node (Application reset)", &table);
            table_print_row("0x82", " ", "Reset communication", &table);
            table_print_footer(&table);
            table_flush(&table);
        }
    }

    return status;
}

void nmt_print_error(const char* reason, nmt_command_t command, disp_mode_t disp_mode)
{
    switch (disp_mode)
    {
        case TERM_MODE:
        {
            os_log(LOG_ERROR, "NMT 0x%02X error: %s", command, reason);
            break;
        }
        case SCRIPT_MODE:
        {
            os_print(LIGHT_BLACK, "NMT  ");
            os_print(DEFAULT_COLOR, "    -       -       -         -       ");
            os_print(LIGHT_RED, "FAIL    ");
            os_print(DARK_MAGENTA, "0x%02X %s\n", command, reason);
            break;
        }
        default:
        case SILENT:
            break;
    }
}
