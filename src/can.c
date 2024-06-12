/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "can.h"
#include "core.h"
#include "printf.h"
#include "table.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <libsocketcan.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
static int can_socket;
#else
#include "PCANBasic.h"
#endif

static int can_monitor(void* core);

void can_init(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->can_monitor_th = SDL_CreateThread(can_monitor, "CAN monitor thread", (void*)core);
}

void can_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->can_status = 0;
    core->is_can_initialised = SDL_FALSE;

#ifdef __linux__
    close(can_socket);
#else
    CAN_Uninitialize(PCAN_USBBUS1);
#endif
}

void can_quit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (SDL_TRUE == is_can_initialised(core))
    {
        can_deinit(core);
    }

    SDL_DetachThread(core->can_monitor_th);
}

Uint32 can_write(can_message_t* message)
{
    int index;

#ifdef __linux__
    struct can_frame frame;
    frame.can_id = message->id;
    frame.can_dlc = message->length;

    for (index = 0; index < 8; index += 1)
    {
        frame.data[index] = message->data[index];
    }

    return write(can_socket, &frame, sizeof(frame));
#else
    TPCANMsg pcan_message = { 0 };

    pcan_message.ID = message->id;
    pcan_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    pcan_message.LEN = message->length;

    for (index = 0; index < 8; index += 1)
    {
        pcan_message.DATA[index] = message->data[index];
    }

    return (Uint32)CAN_Write(PCAN_USBBUS1, &pcan_message);
#endif
}

Uint32 can_read(can_message_t* message)
{
    int index;
#ifdef __linux__
    struct can_frame frame;
    int nbytes = read(can_socket, &frame, sizeof(frame));

    if (nbytes < 0)
    {
        return -1;
    }

    message->id = frame.can_id;
    message->length = frame.can_dlc;

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = frame.data[index];
    }

    return 0;
#else
    Uint32 can_status;
    TPCANMsg pcan_message = { 0 };

    can_status = CAN_Read(PCAN_USBBUS1, &pcan_message, NULL);

    message->id = pcan_message.ID;
    message->length = pcan_message.LEN;

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = pcan_message.DATA[index];
    }

    return can_status;
#endif
}

void can_set_baud_rate(Uint8 command, core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->baud_rate = command;

    if (SDL_TRUE == is_can_initialised(core))
    {
        can_deinit(core);
    }
}

int lua_can_write(lua_State* L)
{
    int    can_id = luaL_checkinteger(L, 1);
    int    length = luaL_checkinteger(L, 2);
    Uint32 data_d0_d3 = luaL_checkinteger(L, 3);
    Uint32 data_d4_d7 = luaL_checkinteger(L, 4);
    can_message_t message = { 0 };

    message.id = can_id;
    message.length = length;

    message.data[3] = (data_d0_d3 & 0xff);
    message.data[2] = ((data_d0_d3 >> 8) & 0xff);
    message.data[1] = ((data_d0_d3 >> 16) & 0xff);
    message.data[0] = ((data_d0_d3 >> 24) & 0xff);
    message.data[7] = (data_d4_d7 & 0xff);
    message.data[6] = ((data_d4_d7 >> 8) & 0xff);
    message.data[5] = ((data_d4_d7 >> 16) & 0xff);
    message.data[4] = ((data_d4_d7 >> 24) & 0xff);

    if (0 == can_write(&message))
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}

void lua_register_can_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_can_write);
    lua_setglobal(core->L, "can_write");
}

void can_print_error_message(const char* context, Uint32 can_status, SDL_bool show_output)
{
    if (SDL_FALSE == show_output)
    {
        return;
    } 

#ifndef __linux__
    if (PCAN_ERROR_OK != can_status)
    {
        char err_message[100] = { 0 };

        CAN_GetErrorText(can_status, 0x09, err_message);
        if (NULL == context)
        {
            c_log(LOG_WARNING, "%s", err_message);
        }
        else
        {
            c_log(LOG_WARNING, "%s: %s", context, err_message);
        }
    }
#else
    // Handle libsocketcan error messages if needed.
#endif
}

void can_print_baud_rate_help(core_t* core)
{
    table_t      table = { DARK_CYAN, DARK_WHITE, 3, 13, 6 };
    char         status[14][7] = { 0 };
    unsigned int status_index = core->baud_rate;
    unsigned int index;

    if (status_index > 13)
    {
        status_index = 13;
    }

    for (index = 0; index < 14; index += 1)
    {
        if (status_index == index)
        {
            SDL_snprintf(status[index], 7, "Active");
        }
        else
        {
            SDL_snprintf(status[index], 2, " ");
        }
    }

    table_print_header(&table);
    table_print_row("CMD", "Description", "Status", &table);
    table_print_divider(&table);
    table_print_row("  0", "1 MBit/s", status[0], &table);
    table_print_row("  1", "800 kBit/s", status[1], &table);
    table_print_row("  2", "500 kBit/s", status[2], &table);
    table_print_row("  3", "250 kBit/s", status[3], &table);
    table_print_row("  4", "125 kBit/s", status[4], &table);
    table_print_row("  5", "100 kBit/s", status[5], &table);
    table_print_row("  6", "95,238 kBit/s", status[6], &table);
    table_print_row("  7", "83,333 kBit/s", status[7], &table);
    table_print_row("  8", "50 kBit/s", status[8], &table);
    table_print_row("  9", "47,619 kBit/s", status[9], &table);
    table_print_row(" 10", "33,333 kBit/s", status[10], &table);
    table_print_row(" 11", "20 kBit/s", status[11], &table);
    table_print_row(" 12", "10 kBit/s", status[12], &table);
    table_print_row(" 13", "5 kBit/s", status[13], &table);
    table_print_footer(&table);
}

SDL_bool is_can_initialised(core_t* core)
{
    if (NULL == core)
    {
        return SDL_FALSE;
    }

    return core->is_can_initialised;
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

    while (SDL_TRUE == core->is_running)
    {
        while (SDL_FALSE == is_can_initialised(core))
        {
#ifdef __linux__
            struct sockaddr_can addr;
            struct ifreq ifr;

            can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
            if (can_socket < 0)
            {
                c_log(LOG_ERROR, "Error while opening socket");
                return 1;
            }

            strcpy(ifr.ifr_name, core->can_interface);
            if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0)
            {
                c_log(LOG_ERROR, "Invalid CAN interface: %s", core->can_interface);
                close(can_socket);
                core->is_can_initialised = SDL_FALSE;
                return 1;
            }

            addr.can_family  = AF_CAN;
            addr.can_ifindex = ifr.ifr_ifindex;

            if (bind(can_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            {
                c_log(LOG_ERROR, "Error in socket bind");
                return 1;
            }

            core->is_can_initialised = SDL_TRUE;
            c_log(LOG_SUCCESS, "CAN successfully initialised");
            c_print_prompt();
#else
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
                c_log(LOG_SUCCESS, "CAN successfully initialised");
                core->is_can_initialised = SDL_TRUE;
                c_print_prompt();
            }
#endif
            SDL_Delay(1);
            continue;
        }

#ifdef __linux__

// Temporarily disabled because until a different solution is found.
// Continously reading CAN frames, leads to a SDO timeout.
#if 0
        struct can_frame frame;
        int nbytes = read(can_socket, &frame, sizeof(frame));

        if (nbytes < 0)
        {
            core->can_status = 0;
            core->is_can_initialised = SDL_FALSE;
            c_log(LOG_WARNING, "CAN de-initialised: Error in CAN read?");
            c_print_prompt();
        }
#endif
#else
        core->can_status = CAN_GetStatus(PCAN_USBBUS1);

        if (PCAN_ERROR_ILLHW == core->can_status)
        {
            core->can_status = 0;
            core->is_can_initialised = SDL_FALSE;

            CAN_Uninitialize(PCAN_USBBUS1);
            c_log(LOG_WARNING, "CAN de-initialised: USB-dongle removed?");
            c_print_prompt();
        }
#endif
        SDL_Delay(1);
    }

    return 0;
}
