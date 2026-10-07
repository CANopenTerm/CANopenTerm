/** @file test_nmt.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_NMT_H
#define TEST_NMT_H

/* NMT help and validation tests */
void test_nmt_print_help(void** state);
void test_nmt_send_command_invalid(void** state);

/* NOTE: nmt_send_command() performs actual CAN communication and is tested through integration tests */

#endif /* TEST_NMT_H */
