/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <errno.h>
#include <fcntl.h>
#include <libsocketcan.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "buffer.h"
#include "can.h"
#include "core.h"
#include "os.h"
#include "table.h"

#define BUFFER_SIZE 8192

static int can_socket;

static int    can_monitor(void* core);
static void   parse_rtattr(struct rtattr* tb[], int max, struct rtattr* rta, int len);
static char** get_can_interfaces(int* count);

status_t can_init(core_t* core)
{
    if (NULL == core)
    {
        return OS_INVALID_ARGUMENT;
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

    core->can_status         = 0;
    core->is_can_initialised = IS_FALSE;

    close(can_socket);
}

void can_flush(void)
{
    /* To be implemented. */
}

status_t can_print_baud_rate_help(core_t* core)
{
    buffer_init(1024);
    os_print(LIGHT_RED, "\nWarning: ");
    os_print(DEFAULT_COLOR, "Setting the CAN interface baud rate requires root permissions.\n");
    os_print(DEFAULT_COLOR, "Please set the baud rate manually using the following command:\n\n");
    os_print(DEFAULT_COLOR, "  sudo ip link set %s up type can bitrate 250000\n\n", core->can_interface);
    os_print(DEFAULT_COLOR, "Replace '250000' with the desired baud rate.\n\n");
    buffer_flush();
    buffer_free();

    return ALL_OK;
}

status_t can_print_channel_help(core_t* core)
{
    status_t     status;
    table_t      table             = {DARK_CYAN, DARK_WHITE, 3, 30, 6};
    char         ch_status[128][7] = {0};
    unsigned int ch_status_index   = core->can_channel;
    unsigned int index;
    int          interface_count;
    char**       can_interfaces = get_can_interfaces(&interface_count);
    int          i;

    if (0 == interface_count)
    {
        os_log(LOG_WARNING, "No CAN hardware found.");
        return CAN_NO_HARDWARE_FOUND;
    }
    else if (interface_count > 127)
    {
        interface_count = 127;
    }

    if (ch_status_index > 127)
    {
        ch_status_index = 127;
    }

    for (index = 0; index < interface_count; index += 1)
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
        char row_desc[10] = {0};

        table_print_header(&table);
        table_print_row("Id.", "Description", "Status", &table);
        table_print_divider(&table);

        for (i = 0; i < interface_count; i += 1)
        {
            os_snprintf(row_index, 4, "%3u", i);
            os_snprintf(row_desc, 10, "%s", can_interfaces[i]);
            table_print_row(row_index, row_desc, ch_status[i], &table);
        }

        table_print_footer(&table);
        table_flush(&table);
    }

    for (i = 0; i < interface_count; i += 1)
    {
        os_free(can_interfaces[i]);
    }
    os_free(can_interfaces);
}

void can_set_baud_rate(uint8 baud_rate_index, core_t* core)
{
    can_print_baud_rate_help(core);
}

void can_set_channel(uint32 channel, core_t* core)
{
    int    interface_count, i;
    char** can_interfaces = get_can_interfaces(&interface_count);

    if (can_interfaces != NULL)
    {
        if (channel < interface_count)
        {
            os_strlcpy(core->can_interface, can_interfaces[channel], 16);
            core->can_channel = channel;

            if (IS_TRUE == is_can_initialised(core))
            {
                can_deinit(core);
            }
        }
        else
        {
            can_print_channel_help(core);
        }
    }

    for (i = 0; i < interface_count; i += 1)
    {
        os_free(can_interfaces[i]);
    }
    os_free(can_interfaces);
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
    core->can_monitor_th = NULL;
}

uint32 can_write(can_message_t* message, disp_mode_t disp_mode, const char* comment)
{
    int              index;
    struct can_frame frame;
    long             num_bytes;

    frame.can_id  = message->id;
    frame.can_dlc = message->length;
    frame.can_id |= message->is_extended ? CAN_EFF_FLAG : 0;

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
    int              index;
    struct can_frame frame;
    struct msghdr    msg;
    struct iovec     iov;
    char             ctrlmsg[CMSG_SPACE(sizeof(struct timeval))];
    struct cmsghdr*  cmsg;
    struct timeval*  tv;
    int              nbytes;

    iov.iov_base = &frame;
    iov.iov_len  = sizeof(frame);

    msg.msg_name       = NULL;
    msg.msg_namelen    = 0;
    msg.msg_iov        = &iov;
    msg.msg_iovlen     = 1;
    msg.msg_control    = &ctrlmsg;
    msg.msg_controllen = sizeof(ctrlmsg);
    msg.msg_flags      = 0;

    nbytes = recvmsg(can_socket, &msg, 0);
    if (nbytes < 0)
    {
        return nbytes;
    }

    message->id          = frame.can_id;
    message->length      = frame.can_dlc;
    message->is_extended = frame.can_id & CAN_EFF_FLAG;

    for (index = 0; index < 8; index += 1)
    {
        message->data[index] = frame.data[index];
    }

    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMP)
        {
            tv                    = (struct timeval*)CMSG_DATA(cmsg);
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
            struct ifreq        ifr;
            int                 buffer_size      = 1024 * 1024; /* 1MB */
            int                 enable_timestamp = 1;

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

            if (IS_FALSE == core->is_plain_mode)
            {
                os_print(DEFAULT_COLOR, "\r");
                os_log(LOG_SUCCESS, "CAN successfully initialised on %s", core->can_interface);
                os_print_prompt();
            }

            os_delay(1);
            continue;
        }

        os_delay(1);
    }

    return 0;
}

void parse_rtattr(struct rtattr* tb[], int max, struct rtattr* rta, int len)
{
    os_memset(tb, 0, sizeof(struct rtattr*) * (max + 1));
    while (RTA_OK(rta, len))
    {
        if (rta->rta_type <= max)
        {
            tb[rta->rta_type] = rta;
        }
        rta = RTA_NEXT(rta, len);
    }
}

static char** get_can_interfaces(int* count)
{
    struct sockaddr_nl sa;
    int                fd;
    char               buf[BUFFER_SIZE] = {0};
    struct iovec       iov              = {buf, sizeof(buf)};
    struct msghdr      msg              = {0};
    int                len;
    int                max_interfaces = 10;
    int                can_count      = 0;
    char**             can_interfaces = (char**)os_calloc(max_interfaces * sizeof(char*), sizeof(char));

    msg.msg_name    = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov     = &iov;
    msg.msg_iovlen  = 1;

    struct
    {
        struct nlmsghdr  nlh;
        struct ifinfomsg ifm;
    } req;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0)
    {
        exit(EXIT_FAILURE);
    }

    os_memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;

    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }

    os_memset(&req, 0, sizeof(req));
    req.nlh.nlmsg_len   = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.nlh.nlmsg_type  = RTM_GETLINK;
    req.ifm.ifi_family  = AF_UNSPEC;

    if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0)
    {
        close(fd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        struct nlmsghdr* nlh;

        len = recvmsg(fd, &msg, 0);
        if (len < 0)
        {
            close(fd);
            exit(EXIT_FAILURE);
        }
        if (0 == len)
        {
            break;
        }

        for (nlh = (struct nlmsghdr*)buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len))
        {
            struct ifinfomsg* ifi;
            struct rtattr*    rta;
            int               rta_len;
            struct rtattr*    tb[IFLA_MAX + 1];

            if (NLMSG_DONE == nlh->nlmsg_type)
            {
                goto out;
            }
            if (nlh->nlmsg_type == NLMSG_ERROR)
            {
                close(fd);
                exit(EXIT_FAILURE);
            }

            ifi     = NLMSG_DATA(nlh);
            rta     = (struct rtattr*)((char*)ifi + NLMSG_ALIGN(sizeof(struct ifinfomsg)));
            rta_len = IFLA_PAYLOAD(nlh);

            parse_rtattr(tb, IFLA_MAX, rta, rta_len);

            if (tb[IFLA_IFNAME])
            {
                char* iface_name = (char*)RTA_DATA(tb[IFLA_IFNAME]);
                if ((0 == os_strncmp(iface_name, "can", 3)) || (0 == os_strncmp(iface_name, "vcan", 4)))
                {
                    if (can_count >= max_interfaces)
                    {
                        max_interfaces *= 2;
                        can_interfaces = (char**)os_realloc(can_interfaces, max_interfaces * sizeof(char*));
                    }
                    can_interfaces[can_count] = strdup(iface_name);
                    can_count++;
                }
            }
        }
    }

out:
    close(fd);
    *count = can_count;

    return can_interfaces;
}
