/** @file can_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <windows.h>
#include "can.h"
#include "core.h"
#include "PCANBasic.h"

static int  can_channel       = 0;
static int  can_channel_index = 0;
static int  can_monitor(void* core);
static char err_message[100]  = { 0 };

void can_init(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->can_monitor_th = os_create_thread(can_monitor, "CAN monitor thread", (void*)core);
}

void can_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->can_status = 0;
    core->is_can_initialised = IS_FALSE;

    CAN_Uninitialize(can_channel);
}

void can_quit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (IS_TRUE == is_can_initialised(core))
    {
        can_deinit(core);
    }

    os_detach_thread(core->can_monitor_th);
}

uint32 can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    int      index;
    TPCANMsg pcan_message = { 0 };

    pcan_message.ID      = message->id;
    pcan_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    pcan_message.LEN     = message->length;

    for (index = 0; index < 8; index += 1)
    {
        pcan_message.DATA[index] = message->data[index];
    }

    return (uint32)CAN_Write(can_channel, &pcan_message);
}

uint32 can_read(can_message_t* message)
{
    int            index;
    uint32         can_status;
    TPCANMsg       pcan_message   = { 0 };
    TPCANTimestamp pcan_timestamp = { 0 };

    can_status = CAN_Read(can_channel, &pcan_message, &pcan_timestamp);

    message->id           = pcan_message.ID;
    message->length       = pcan_message.LEN;
    message->timestamp_us =
        pcan_timestamp.micros
        + (1000ULL * pcan_timestamp.millis)
        + (0x100000000ULL * 1000ULL * pcan_timestamp.millis_overflow);
        
    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = pcan_message.DATA[index];
    }

    return can_status;
}

void can_set_baud_rate(uint8 command, core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->baud_rate = command;

    if (IS_TRUE == is_can_initialised(core))
    {
        can_deinit(core);
    }
}

void can_set_channel(int channel, core_t* core)
{
    (void)core;

    if (channel >= PCAN_PCIBUS16)
    {
        channel = PCAN_PCIBUS16;
    }

    can_channel_index = channel;
    can_deinit(core);
}

const char* can_get_error_message(uint32 can_status)
{
    if (PCAN_ERROR_OK != can_status)
    {
        CAN_GetErrorText(can_status, 0x09, err_message);
        return err_message;
    }
    else
    {
        return "Unknown CAN error or not handled";
    }
}

static int can_monitor(void* core_pt)
{
    char    err_message[100] = { 0 };
    core_t* core             = core_pt;

    TPCANBaudrate baud_rates[] = {
        PCAN_BAUD_1M,
        PCAN_BAUD_800K,
        PCAN_BAUD_500K,
        PCAN_BAUD_250K,
        PCAN_BAUD_125K,
        PCAN_BAUD_100K,
        PCAN_BAUD_95K,
        PCAN_BAUD_83K,
        PCAN_BAUD_50K,
        PCAN_BAUD_47K,
        PCAN_BAUD_33K,
        PCAN_BAUD_20K,
        PCAN_BAUD_10K,
        PCAN_BAUD_5K
    };

    const char* baud_rate_desc[] = {
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
        "5 kBit/s"
    };

    TPCANHandle can_channels[] = {
        PCAN_USBBUS1,
        PCAN_USBBUS2,
        PCAN_USBBUS3,
        PCAN_USBBUS4,
        PCAN_USBBUS5,
        PCAN_USBBUS6,
        PCAN_USBBUS7,
        PCAN_USBBUS8,
        PCAN_USBBUS9,
        PCAN_USBBUS10,
        PCAN_USBBUS11,
        PCAN_USBBUS12,
        PCAN_USBBUS13,
        PCAN_USBBUS14,
        PCAN_USBBUS15,
        PCAN_USBBUS16,
        PCAN_PCIBUS1,
        PCAN_PCIBUS2,
        PCAN_PCIBUS3,
        PCAN_PCIBUS4,
        PCAN_PCIBUS5,
        PCAN_PCIBUS6,
        PCAN_PCIBUS7,
        PCAN_PCIBUS8,
        PCAN_PCIBUS9,
        PCAN_PCIBUS10,
        PCAN_PCIBUS11,
        PCAN_PCIBUS12,
        PCAN_PCIBUS13,
        PCAN_PCIBUS14,
        PCAN_PCIBUS15,
        PCAN_PCIBUS16
    };

    const char* can_channel_desc[] = {
        "PCAN-USB channel 1",
        "PCAN-USB channel 2",
        "PCAN-USB channel 3",
        "PCAN-USB channel 4",
        "PCAN-USB channel 5",
        "PCAN-USB channel 6",
        "PCAN-USB channel 7",
        "PCAN-USB channel 8",
        "PCAN-USB channel 9",
        "PCAN-USB channel 10",
        "PCAN-USB channel 11",
        "PCAN-USB channel 12",
        "PCAN-USB channel 13",
        "PCAN-USB channel 14",
        "PCAN-USB channel 15",
        "PCAN-USB channel 16",
        "PCAN-PCI interface, channel 1",
        "PCAN-PCI interface, channel 2",
        "PCAN-PCI interface, channel 3",
        "PCAN-PCI interface, channel 4",
        "PCAN-PCI interface, channel 5",
        "PCAN-PCI interface, channel 6",
        "PCAN-PCI interface, channel 7",
        "PCAN-PCI interface, channel 8",
        "PCAN-PCI interface, channel 9",
        "PCAN-PCI interface, channel 10",
        "PCAN-PCI interface, channel 11",
        "PCAN-PCI interface, channel 12",
        "PCAN-PCI interface, channel 13",
        "PCAN-PCI interface, channel 14",
        "PCAN-PCI interface, channel 15",
        "PCAN-PCI interface, channel 16"
    };

    if (NULL == core)
    {
        return 1;
    }

    while (IS_TRUE == core->is_running)
    {
        while (IS_FALSE == is_can_initialised(core))
        {
            int num_baud_rates = sizeof(baud_rates) / sizeof(baud_rates[0]);
            int num_can_channels = sizeof(can_channels) / sizeof(can_channels[0]);
            int chan_i;
            int rate_i;

            for (chan_i = can_channel_index; chan_i < num_can_channels; chan_i++)
            {
                for (rate_i = 0; rate_i < num_baud_rates; rate_i++)
                {
                    TPCANBaudrate baud_rate = baud_rates[rate_i];
                    TPCANHandle   channel = can_channels[chan_i];

                    core->can_status = CAN_Initialize(
                        channel,
                        baud_rate,
                        PCAN_NONE,
                        0, 0);

                    CAN_GetErrorText(core->can_status, 0x09, err_message);

                    if ((core->can_status & PCAN_ERROR_OK) == core->can_status)
                    {
                        os_print(DEFAULT_COLOR, "\r");
                        os_log(LOG_SUCCESS, "CAN successfully initialised on %s with baud rate %s", can_channel_desc[chan_i], baud_rate_desc[rate_i]);
                        core->is_can_initialised = IS_TRUE;
                        core->baud_rate          = rate_i;
                        can_channel              = channel;
                        can_channel_index        = chan_i;
                        os_print_prompt();
                        break;
                    }

                    if (core->is_can_initialised == IS_TRUE)
                    {
                        break;
                    }
                }

                if (core->is_can_initialised == IS_TRUE)
                {
                    break;
                }
            }
            os_delay(1);
        }

        core->can_status = CAN_GetStatus(can_channel);

        if (PCAN_ERROR_ILLHW == core->can_status)
        {
            core->can_status         = 0;
            core->is_can_initialised = IS_FALSE;

            can_deinit(core);
            os_log(LOG_WARNING, "CAN de-initialised: USB-dongle removed?");
            os_print_prompt();
        }
        os_delay(1);
    }

    return 0;
}
