/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <CANvenient.h>

#include "buffer.h"
#include "can.h"
#include "core.h"
#include "os.h"
#include "table.h"

static const char* baud_rate_desc[] = {
    "1 MBit/s",
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

static int can_monitor(void* core);
static void find_can_channel(core_t* core, enum can_baudrate baud);

void limit_node_id(uint8* node_id)
{
    if (*node_id > 0x7f)
    {
        *node_id = 0x7f;
    }
}

bool is_can_initialised(core_t* core)
{
    if (NULL == core)
    {
        return false;
    }

    return core->is_can_initialised;
}

void can_print_error(uint32 can_id, const char* reason, disp_mode_t disp_mode)
{
    if (SCRIPT_MODE != disp_mode)
    {
        return;
    }

    os_print(LIGHT_BLACK, "CAN ");
    os_print(DEFAULT_COLOR, "     0x%02X   -       -         -       ", can_id);
    os_print(LIGHT_RED, "FAIL    ");

    if (NULL != reason)
    {
        os_print(DEFAULT_COLOR, "%s\n", reason);
    }
    else
    {
        os_print(DEFAULT_COLOR, "-\n");
    }
}

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
    can_close(core->can_channel);
    can_open(core->can_channel, core->baud_rate);
}

status_t can_print_baud_rate_help(core_t* core)
{
    status_t status;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 13, 6};
    char br_status[15][7] = {0};
    unsigned int br_status_index = core->baud_rate;
    unsigned int index;

    if (br_status_index >= 14)
    {
        br_status_index = 14;
    }

    for (index = 1; index <= 14; index += 1)
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

        for (i = 1; i <= 14; i += 1)
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
    status_t status = ALL_OK;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 48, 6};
    char ch_status[33][7] = {0};
    unsigned int ch_status_index = core->can_channel;
    unsigned int ch_count = 0;
    int indices[CAN_MAX_INTERFACES] = {0};
    int i;

    for (i = 0; i < CAN_MAX_INTERFACES; i++)
    {
        if (0 == can_update(i))
        {
            indices[ch_count] = i;
            ch_count++;
        }
    }

    if (0 == ch_count)
    {
        os_log(LOG_WARNING, "No CAN hardware found.");
        return CAN_NO_HARDWARE_FOUND;
    }

    for (i = 0; i < ch_count; i++)
    {
        if (ch_status_index == i)
        {
            os_snprintf(ch_status[indices[i]], 7, "Active");
        }
        else
        {
            os_snprintf(ch_status[indices[i]], 2, " ");
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

        for (i = 0; i < ch_count; i++)
        {
            char name_buf[48] = {0};
            enum can_baudrate baud = 0;

            can_get_name(indices[i], name_buf, sizeof(name_buf));
            can_get_baudrate(indices[i], &baud);

            os_snprintf(row_index, 4, "%3u", indices[i]);
            if (ch_status_index == i)
            {
                os_snprintf(row_desc, 48, "%s (%s)", name_buf, baud_rate_desc[baud + 1]);
            }
            else
            {
                os_snprintf(row_desc, 48, "%s", name_buf);
            }
            table_print_row(row_index, row_desc, ch_status[i], &table);
        }

        table_print_footer(&table);
        table_flush(&table);
    }
    return status;
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

    os_detach_thread(core->can_monitor_th);
    core->can_monitor_th = NULL;
}

uint32 can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    struct can_frame frame = {0};
    int i;

    /* Not yet implemented. */
    (void)disp_mode;
    (void)comment;

    frame.can_id = message->id;
    frame.can_dlc = message->length;

    for (i = 0; i < 8; i++)
    {
        frame.data[i] = message->data[i];
    }

    if (0 == can_send(core->can_channel, &frame))
    {
        return ALL_OK;
    }
    else
    {
        return CAN_WRITE_ERROR;
    }
}

uint32 can_read(can_message_t* message)
{
    uint32 can_status;
    u64 timestamp;
    struct can_frame frame = {0};

    can_status = can_recv(core->can_channel, &frame, &timestamp);
    if (0 == can_status)
    {
        int i;
        message->id = frame.can_id;
        message->length = frame.can_dlc;
        message->timestamp_us = timestamp;

        for (i = 0; i < 8; i++)
        {
            message->data[i] = frame.data[i];
        }

        return ALL_OK;
    }
    else
    {
        return CAN_READ_ERROR;
    }
}

void can_set_baud_rate(uint8 baud_rate_index, core_t* core)
{
#ifdef _WIN32

    if (NULL == core)
    {
        return;
    }

    if (0 == baud_rate_index)
    {
        baud_rate_index = 1;
    }

    if (baud_rate_index >= 14)
    {
        can_print_baud_rate_help(core);
        return;
    }

    core->baud_rate = baud_rate_index;
    can_set_baudrate(core->can_channel, (enum can_baudrate)(baud_rate_index - 1));

    if (true == is_can_initialised(core))
    {
        can_deinit(core);
    }

#elif __linux__

    (void)baud_rate_index;
    (void)core;

    buffer_init(1024);
    os_print(LIGHT_RED, "\nWarning: ");
    os_print(DEFAULT_COLOR, "Setting the CAN interface baud rate requires root permissions.\n");
    os_print(DEFAULT_COLOR, "Please set the baud rate manually using the following command:\n\n");
    os_print(DEFAULT_COLOR, "  sudo ip link set %s up type can bitrate 250000\n\n", core->can_interface);
    os_print(DEFAULT_COLOR, "Replace '250000' with the desired baud rate.\n\n");
    buffer_flush();
    buffer_free();

#endif
}

void can_set_channel(uint32 channel, core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (can_update((int)channel) < 0)
    {
        can_print_channel_help(core);
        return;
    }

    if (true == is_can_initialised(core))
    {
        can_deinit(core);
    }

    core->can_channel = channel;
    core->set_can_channel = true;
}

const char* can_get_error_message(uint32 can_status)
{
    static char err_message[1024];

    can_get_error(err_message, sizeof(err_message));
    return err_message;
}

static int can_monitor(void* core_pt)
{
    core_t* core = core_pt;

    if (NULL == core)
    {
        return 1;
    }

    find_can_channel(core, CAN_BAUD_1M);

    while (true == core->is_running)
    {
        while (false == is_can_initialised(core))
        {
            find_can_channel(core, (enum can_baudrate)(core->baud_rate - 1));
            os_delay(1);
        }

        if (can_update(core->can_channel) < 0)
        {
            can_deinit(core);
            os_print(DEFAULT_COLOR, "\n");
            os_log(LOG_WARNING, "CAN de-initialised: Hardware removed?");
            os_print_prompt();
        }
        os_delay(1);
    }

    return 0;
}

static void find_can_channel(core_t* core, enum can_baudrate baud)
{
    int i;
    can_find_interfaces();

    for (i = 0; i < CAN_MAX_INTERFACES; i++)
    {
        int ch = ((int)core->can_channel + i) % CAN_MAX_INTERFACES;

        if (0 == can_open(ch, baud))
        {
            char name_buf[256] = {0};

            can_get_name(ch, name_buf, sizeof(name_buf));

            os_print(DEFAULT_COLOR, "\r");
            os_log(LOG_SUCCESS, "CAN successfully initialised on %s with baud rate %s\n", name_buf, baud_rate_desc[(int)baud + 1]);
            os_print_prompt();

            core->is_can_initialised = true;
            core->can_channel = ch;
            break;
        }
        else
        {
            /* Show error message when switching to a new channel fails. */
            if (core->set_can_channel)
            {
                char reason_buf[1024] = {0};
                can_get_error(reason_buf, sizeof(reason_buf));

                os_log(LOG_WARNING, "%s", reason_buf);
                core->set_can_channel = false;
            }
        }
    }
}
