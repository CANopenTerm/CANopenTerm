/** @file can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "os.h"

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
