/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "os.h"
#include "pdo.h"
#include "table.h"

static pdo_t pdo[PDO_MAX];

static uint32 pdo_send_callback(uint32 interval, void *param);
static void   print_error(const char* reason, disp_mode_t disp_mode, uint16 can_id);

bool_t pdo_add(uint16 can_id, uint32 event_time_ms, uint8 length, uint64 data, disp_mode_t disp_mode)
{
    int i;

    if (IS_FALSE == pdo_is_id_valid(can_id))
    {
        print_error("Could not add PDO: Invalid TPDO CAN-ID", disp_mode, can_id);
        return IS_FALSE;
    }

    if (length > 8)
    {
        length = 8;
    }

    if (IS_FALSE == pdo_del(can_id, disp_mode))
    {
        /* Nothing to do here. */
    }

    for (i = 0; i < PDO_MAX; i += 1)
    {
        if (0 == pdo[i].can_id)
        {
            pdo[i].can_id = can_id;
            pdo[i].length = length;
            pdo[i].data   = data;
            pdo[i].id     = os_add_timer(event_time_ms, pdo_send_callback, &pdo[i]);
            return IS_TRUE;
        }
    }

    print_error("Could not add PDO: No empty slot available", disp_mode, can_id);
    return IS_FALSE;
}

bool_t pdo_del(uint16 can_id, disp_mode_t disp_mode)
{
    int i;

    if (IS_FALSE == pdo_is_id_valid(can_id))
    {
        print_error("Could not delete PDO: Invalid TPDO CAN-ID", disp_mode, can_id);
        return IS_FALSE;
    }

    for (i = 0; i < PDO_MAX; i += 1)
    {
        if (can_id == pdo[i].can_id)
        {
            pdo[i].can_id = 0;
            pdo[i].length = 0;
            pdo[i].data   = 0;

            os_remove_timer(pdo[i].id);
            break;
        }
    }

    return IS_TRUE;
}

static uint32 pdo_send_callback(uint32 interval, void *pdo_pt)
{
    int           i;
    int           offset  = 0;
    pdo_t*        pdo     = pdo_pt;
    can_message_t message = { 0 };

    message.id     = pdo->can_id;
    message.length = pdo->length;

    for (i = (pdo->length - 1); i >= 0; i -= 1)
    {
        message.data[i] = ((pdo->data >> offset) & 0xFF);
        offset += 8;
    }

    can_write(&message, SILENT, NULL);

    return interval;
}

status_t pdo_print_help(void)
{
    status_t status;
    table_t  table = { DARK_CYAN, DARK_WHITE, 13, 7, 7 };

    status = table_init(&table, 1024);
    if (ALL_OK == status)
    {
        table_print_header(&table);
        table_print_row("CAN-ID",        "Object",  "Spec.",   &table);
        table_print_divider(&table);
        table_print_row("0x000 - 0x07f", "Node-ID", " ",       &table);
        table_print_row("0x181 - 0x1ff", "TPDO1",   "CiA 301", &table);
        table_print_row("0x281 - 0x1ff", "TPDO2",   "CiA 301", &table);
        table_print_row("0x381 - 0x1ff", "TPDO3",   "CiA 301", &table);
        table_print_row("0x481 - 0x1ff", "TPDO4",   "CiA 301", &table);
        table_print_footer(&table);
        table_flush(&table);
    }

    return status;
}

bool_t pdo_is_id_valid(uint16 can_id)
{
    /* Node-ID (0x000 - 0x07f)
     * TPDO 1  (0x181 - 0x1ff)
     * TPDO 2  (0x281 - 0x2ff)
     * TPDO 3  (0x381 - 0x3ff)
     * TPDO 4  (0x481 - 0x4ff)
     */
    if ((can_id >= 0x00) && (can_id <= 0x7f))
    {
        return IS_TRUE;
    }
    else if ((can_id >= 0x180) && (can_id <= 0x4ff))
    {
        switch (can_id)
        {
            case 0x180:
            case 0x280:
            case 0x380:
            case 0x480:
                return IS_FALSE;
            default:
                return IS_TRUE;
        }
    }

    return IS_FALSE;
}

void pdo_print_result(uint16 can_id, uint32 event_time_ms, uint64 data, bool_t was_successful, const char *comment)
{
    int  i;
    char buffer[34] = { 0 };

    if (NULL == comment)
    {
        comment = "-";
    }

    os_strlcpy(buffer, comment, 33);
    for (i = os_strlen(buffer); i < 33; ++i)
    {
        buffer[i] = ' ';
    }

    if (IS_TRUE == was_successful)
    {
        os_print(LIGHT_BLACK, "PDO  ");
        os_print(DEFAULT_COLOR, "    0x%03X   -       -         -       ", can_id);
        os_print(LIGHT_GREEN, "SUCC    ");
        os_print(DARK_MAGENTA, "%s ", buffer);


        if ((0 == data) && (0 == event_time_ms))
        {
            os_print(DEFAULT_COLOR, "Delete\n");
        }
        else
        {
            os_print(DEFAULT_COLOR, "0x%08X, %ums\n", data, event_time_ms);
        }
    }
}

static void print_error(const char* reason, disp_mode_t disp_mode, uint16 can_id)
{
    if (SCRIPT_MODE != disp_mode)
    {
        return;
    }

    os_print(LIGHT_BLACK, "PDO ");
    os_print(DEFAULT_COLOR, "     0x%03X   -       -         -       ", can_id);
    os_print(LIGHT_RED, "FAIL    ");
    os_print(DEFAULT_COLOR, "%s\n", reason);
}
