/** @file can_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <CANvenient.h>

#include "can.h"
#include "core.h"
#include "table.h"

static const char* baud_rate_desc[] = {
    "1 MBit/s",
    "800 kBit/s",
    "500 kBit/s",
    "250 kBit/s",
    "125 kBit/s",
    "100 kBit/s",
    "95.238 kBit/s",
    "83.333 kBit/s",
    "50 kBit/s",
    "47.619 kBit/s",
    "33.333 kBit/s",
    "20 kBit/s",
    "10 kBit/s",
    "5 kBit/s"};

static uint32 pcan_channel_count;
static char err_message[1024] = {0};

static int can_monitor(void* core);
static void search_free_can_configuration(core_t* core, bool search_baud_rate, bool search_channel);

status_t can_init(core_t* core)
{
    status_t status;

    if (NULL == core)
    {
        return OS_INVALID_ARGUMENT;
    }

    can_deinit(core);

    status = can_find_interfaces();
    if (ALL_OK != status)
    {
        return status;
    }

    core->can_monitor_th = os_create_thread(can_monitor, "CAN monitor thread", (void*)core);

    return ALL_OK;
}

void can_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->can_status = 0;
    core->is_can_initialised = false;

    can_close(core->can_channel);
}

void can_flush(void)
{
    // CAN_Reset(peak_can_channel);
}

status_t can_print_baud_rate_help(core_t* core)
{
    status_t status;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 13, 6};
    char br_status[14][7] = {0};
    unsigned int br_status_index = core->baud_rate;
    unsigned int index;

    if (br_status_index >= 13)
    {
        br_status_index = 13;
    }

    for (index = 0; index <= 13; index += 1)
    {
        if (br_status_index == index)
        {
            os_snprintf(br_status[index], 7, "Active");
        }
        else
        {
            os_snprintf(br_status[index], 2, " ");
        }
    }

    status = table_init(&table, 1024);
    if (ALL_OK == status)
    {
        int i;
        char row_index[4] = {0};
        char row_desc[14] = {0};

        table_print_header(&table);
        table_print_row("Id.", "Description", "Status", &table);
        table_print_divider(&table);

        for (i = 0; i <= 13; i += 1)
        {
            os_snprintf(row_index, 4, "%3u", i);
            os_snprintf(row_desc, 14, "%s", baud_rate_desc[i]);
            table_print_row(row_index, row_desc, br_status[i], &table);
        }

        table_print_footer(&table);
        table_flush(&table);
    }

    return status;
}

status_t can_print_channel_help(core_t* core)
{
#if 0
    status_t status;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 30, 6};
    char ch_status[33][7] = {0};
    unsigned int ch_status_index = core->can_channel;
    unsigned int index;

    status = search_can_channels();
    if (ALL_OK != status)
    {
        return status;
    }

    if (0 == pcan_channel_count)
    {
        os_log(LOG_WARNING, "No CAN hardware found.");
        return CAN_NO_HARDWARE_FOUND;
    }

    for (index = 0; index < pcan_channel_count; index += 1)
    {
        if (ch_status_index == index)
        {
            os_snprintf(ch_status[index], 7, "Active");
        }
        else
        {
            os_snprintf(ch_status[index], 2, " ");
        }
    }

    status = table_init(&table, 1024);
    if (ALL_OK == status)
    {
        char row_index[4] = {0};
        char row_desc[48] = {0};

        table_print_header(&table);
        table_print_row("Id.", "Description", "Status", &table);
        table_print_divider(&table);

        for (index = 0; index < pcan_channel_count; index += 1)
        {
            os_snprintf(row_index, 4, "%3u", index);
            if (ch_status_index == index)
            {
                os_snprintf(row_desc, 48, "%s (%s)", pcan_channel_information[index].device_name, baud_rate_desc[core->baud_rate]);
            }
            else
            {
                os_snprintf(row_desc, 32, "%s", pcan_channel_information[index].device_name);
            }
            table_print_row(row_index, row_desc, ch_status[index], &table);
        }

        table_print_footer(&table);
        table_flush(&table);
    }

    return status;
#endif
    return 0;
}

void can_quit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (true == is_can_initialised(core))
    {
        can_deinit(core);
    }

    // if (NULL != pcan_channel_information)
    //{
    //     os_free(pcan_channel_information);
    // }

    os_detach_thread(core->can_monitor_th);
    core->can_monitor_th = NULL;
}

uint32 can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
#if 0
    int index;
    TPCANMsg pcan_message = {0};

    /* Not yet implemented. */
    (void)disp_mode;
    (void)comment;

    pcan_message.ID = message->id;
    pcan_message.LEN = message->length;

    if (message->is_extended == true)
    {
        pcan_message.MSGTYPE = PCAN_MESSAGE_EXTENDED;
    }
    else
    {
        pcan_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    }

    for (index = 0; index < 8; index += 1)
    {
        pcan_message.DATA[index] = message->data[index];
    }

    return (uint32)CAN_Write(peak_can_channel, &pcan_message);
#endif
    return 0;
}

uint32 can_read(can_message_t* message)
{
#if 0
    int index;
    uint32 can_status;
    TPCANMsg pcan_message = {0};
    TPCANTimestamp pcan_timestamp = {0};

    can_status = CAN_Read(peak_can_channel, &pcan_message, &pcan_timestamp);

    message->id = pcan_message.ID;
    message->length = pcan_message.LEN;
    message->is_extended = (PCAN_MESSAGE_EXTENDED == pcan_message.MSGTYPE) ? true : false;
    message->timestamp_us =
        pcan_timestamp.micros + (1000ULL * pcan_timestamp.millis) + (0x100000000ULL * 1000ULL * pcan_timestamp.millis_overflow);

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = pcan_message.DATA[index];
    }

    return can_status;
#endif
    return 0;
}

void can_set_baud_rate(uint8 baud_rate_index, core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (baud_rate_index >= 13)
    {
        can_print_baud_rate_help(core);
        return;
    }

    core->baud_rate = baud_rate_index;

    if (true == is_can_initialised(core))
    {
        can_deinit(core);
    }
}

void can_set_channel(uint32 channel, core_t* core)
{
#if 0
    if (NULL == core)
    {
        return;
    }

    if (ALL_OK != search_can_channels())
    {
        can_print_channel_help(core);
        return;
    }

    if (channel >= pcan_channel_count)
    {
        can_print_channel_help(core);
        return;
    }

    core->can_channel = channel;

    if (true == is_can_initialised(core))
    {
        can_deinit(core);
    }
#endif
}

const char* can_get_error_message(uint32 can_status)
{
#if 0
    if (PCAN_ERROR_OK != can_status)
    {
        CAN_GetErrorText(can_status, 0x09, err_message);
        return err_message;
    }
    else
    {
        return "Unknown CAN error or not handled";
    }
#endif
    return "CAN error handling not yet implemented.";
}

static int can_monitor(void* core_pt)
{
    core_t* core = core_pt;

    if (NULL == core)
    {
        return 1;
    }

    search_free_can_configuration(core, true, true);

    while (true == core->is_running)
    {
        enum can_status status = CAN_STATUS_OK;

        while (false == is_can_initialised(core))
        {
            search_free_can_configuration(core, false, false);
            os_delay(1);
        }

        can_get_interface_status(core->can_channel, &status);
        if (CAN_STATUS_INVALID_HARDWARE == status)
        {
            can_deinit(core);
            os_print(DEFAULT_COLOR, "\n");
            os_log(LOG_WARNING, "CAN de-initialised: USB-dongle removed?");
            os_print_prompt();
        }
        os_delay(1);
    }

    return 0;
}

static void search_free_can_configuration(core_t* core, bool search_baud_rate, bool search_channel)
{
    int channel_count = 0;

    can_get_interface_count(&channel_count);
    if (0 == channel_count)
    {
        return;
    }

    for (int i = 0; i < CAN_MAX_INTERFACES; i++)
    {
        if (0 == can_open(i))
        {
            enum can_baudrate baud = CAN_BAUD_1M;
            char name_buf[256] = {0};

            can_get_baudrate(i, &baud);
            can_get_interface_name(i, name_buf, sizeof(name_buf));

            os_print(DEFAULT_COLOR, "\r");
            os_log(LOG_SUCCESS, "CAN successfully initialised on %s with baud rate %s", name_buf, baud_rate_desc[baud]);
            os_print_prompt();

            core->is_can_initialised = true;
            core->can_channel = i;
            break;
        }
    }
}
