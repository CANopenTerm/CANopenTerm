/** @file pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PDO_H
#define PDO_H

#include "core.h"
#include "os.h"

#define PDO_MAX 0x1f8 /* TPDO1 - TPDO4 */

typedef struct pdo
{
    os_timer_id id;
    uint16 can_id;
    uint8 length;
    uint64 data;

} pdo_t;

bool pdo_add(uint16 can_id, uint32 event_time_ms, uint8 length, uint64 data, disp_mode_t disp_mode);
bool pdo_del(uint16 can_id, disp_mode_t disp_mode);
status_t pdo_print_help(void);
bool pdo_is_id_valid(uint16 can_id);
void pdo_print_result(uint16 can_id, uint32 event_time_ms, uint64 data, bool was_successful, const char* comment);

#endif /* PDO_H */
