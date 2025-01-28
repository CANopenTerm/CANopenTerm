/** @file ctt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ctt.h"
#include "codb.h"
#include "eds.h"
#include "test_report.h"

/* Test specification. Draft.
 *
 * SDO test:
 *
 * 1. Check all mandatory CiA 301 objects.
 *
 *
 *
 *
 *
 *
 */

status_t cct_run_test(uint32 node_id)
{
    status_t status = ALL_OK;

    os_log(LOG_INFO, "CANopen CC conformance test.");

    return status;
}
