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

static int  can_monitor(void* core);
static char err_message[100] = { 0 };

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

    CAN_Uninitialize(PCAN_USBBUS1);
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

    return (uint32)CAN_Write(PCAN_USBBUS1, &pcan_message);
}

uint32 can_read(can_message_t* message)
{
    int            index;
    uint32         can_status;
    TPCANMsg       pcan_message   = { 0 };
    TPCANTimestamp pcan_timestamp = { 0 };

    can_status = CAN_Read(PCAN_USBBUS1, &pcan_message, &pcan_timestamp);

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
    core_t* core = core_pt;

    if (NULL == core)
    {
        return 1;
    }

    core->baud_rate = 3;

    while (IS_TRUE == core->is_running)
    {
        while (IS_FALSE == is_can_initialised(core))
        {
            TPCANBaudrate baud_rate;

            switch (core->baud_rate)
            {
            case 0:
                baud_rate = PCAN_BAUD_1M;
                break;
            case 1:
                baud_rate = PCAN_BAUD_800K;
                break;
            case 2:
                baud_rate = PCAN_BAUD_500K;
                break;
            case 3:
            default:
                baud_rate = PCAN_BAUD_250K;
                break;
            case 4:
                baud_rate = PCAN_BAUD_125K;
                break;
            case 5:
                baud_rate = PCAN_BAUD_100K;
                break;
            case 6:
                baud_rate = PCAN_BAUD_95K;
                break;
            case 7:
                baud_rate = PCAN_BAUD_83K;
                break;
            case 8:
                baud_rate = PCAN_BAUD_50K;
                break;
            case 9:
                baud_rate = PCAN_BAUD_47K;
                break;
            case 10:
                baud_rate = PCAN_BAUD_33K;
                break;
            case 11:
                baud_rate = PCAN_BAUD_20K;
                break;
            case 12:
                baud_rate = PCAN_BAUD_10K;
                break;
            case 13:
                baud_rate = PCAN_BAUD_5K;
                break;
            }

            core->can_status = CAN_Initialize(
                PCAN_USBBUS1,
                baud_rate,
                PCAN_USB,
                0, 0);

            CAN_GetErrorText(core->can_status, 0x09, err_message);

            if ((core->can_status & PCAN_ERROR_OK) == core->can_status)
            {
                os_log(LOG_SUCCESS, "CAN successfully initialised");
                core->is_can_initialised = IS_TRUE;
                os_print_prompt();
            }

            os_delay(1);
            continue;
        }

        core->can_status = CAN_GetStatus(PCAN_USBBUS1);

        if (PCAN_ERROR_ILLHW == core->can_status)
        {
            core->can_status = 0;
            core->is_can_initialised = IS_FALSE;

            CAN_Uninitialize(PCAN_USBBUS1);
            os_log(LOG_WARNING, "CAN de-initialised: USB-dongle removed?");
            os_print_prompt();
        }

        os_delay(1);
    }

    return 0;
}
