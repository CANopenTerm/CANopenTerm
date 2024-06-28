/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <errno.h>
#include <fcntl.h>
#include <libsocketcan.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "can.h"
#include "core.h"
#include "os.h"
#include "table.h"

static int can_socket;

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

    close(can_socket);
}

void can_set_baud_rate(uint8 command, core_t* core)
{
    (void)command;
    (void)core;
    os_log(LOG_ERROR, "This feature cannot yet be used on Linux: The baud rate remains unchanged");
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
    int    index;
    struct can_frame frame;
    long   num_bytes;

    frame.can_id  = message->id;
    frame.can_dlc = message->length;

    for (index = 0; index < 8; index += 1)
    {
        frame.data[index] = message->data[index];
    }

    num_bytes = write(can_socket, &frame, sizeof(frame));

    os_delay(1);

    if (-1 == num_bytes)
    {
        return errno;
    }
    else
    {
        return 0;
    }
}

uint32 can_read(can_message_t* message)
{
    int    index;
    struct can_frame frame;
    struct msghdr msg;
    struct iovec iov;
    char   ctrlmsg[CMSG_SPACE(sizeof(struct timeval))];
    struct cmsghdr* cmsg;
    struct timeval* tv;
    int    nbytes;

    iov.iov_base = &frame;
    iov.iov_len = sizeof(frame);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &ctrlmsg;
    msg.msg_controllen = sizeof(ctrlmsg);
    msg.msg_flags = 0;

    nbytes = recvmsg(can_socket, &msg, 0);
    if (nbytes < 0)
    {
        return nbytes;
    }

    message->id = frame.can_id;
    message->length = frame.can_dlc;

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = frame.data[index];
    }

    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP)
        {
            tv = (struct timeval*)CMSG_DATA(cmsg);
            message->timestamp_us = tv->tv_sec * 1000000ULL + tv->tv_usec;
            break;
        }
    }

    return 0;
}

const char* can_get_error_message(uint32 can_status)
{
    /* Handle libsocketcan error messages if needed. */

    return "Unknown CAN error or not handled";
}

static int can_monitor(void* core_pt)
{
    char    err_message[100] = { 0 };
    core_t* core = core_pt;

    if (NULL == core)
    {
        return 1;
    }

    while (IS_TRUE == core->is_running)
    {
        while (IS_FALSE == is_can_initialised(core))
        {
            struct sockaddr_can addr;
            struct ifreq ifr;
            int    buffer_size = 1024 * 1024; /* 1MB */
            int    enable_timestamp = 1;

            can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
            if (can_socket < 0)
            {
                os_log(LOG_ERROR, "Error while opening socket");
                return 1;
            }

            setsockopt(can_socket, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
            setsockopt(can_socket, SOL_SOCKET, SO_TIMESTAMP, &enable_timestamp, sizeof(enable_timestamp));

            strcpy(ifr.ifr_name, core->can_interface);
            if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0)
            {
                os_log(LOG_ERROR, "Invalid CAN interface: %s", core->can_interface);
                close(can_socket);
                core->is_can_initialised = IS_FALSE;
                return 1;
            }

            addr.can_family  = AF_CAN;
            addr.can_ifindex = ifr.ifr_ifindex;

            if (bind(can_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            {
                os_log(LOG_ERROR, "Error in socket bind");
                return 1;
            }

            core->is_can_initialised = IS_TRUE;
            os_print(DEFAULT_COLOR, "\r");
            os_log(LOG_SUCCESS, "CAN successfully initialised");
            os_print_prompt();

            os_delay(1);
            continue;
        }

        os_delay(1);
    }

    return 0;
}
