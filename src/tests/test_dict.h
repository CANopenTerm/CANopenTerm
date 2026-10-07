/** @file test_dict.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_DICT_H
#define TEST_DICT_H

/* Dictionary lookup tests for CANopen object mapping */
void test_dict_lookup(void** state);
void test_dict_lookup_raw(void** state);
void test_dict_lookup_unknown(void** state);
void test_emcy_lookup(void** state);

/* NOTE: dict_lookup_object() is a helper for command validation and integration testing */

#endif /* TEST_DICT_H */
