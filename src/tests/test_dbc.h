/** @file test_dbc.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_DBC_H
#define TEST_DBC_H

void test_dbc_unloaded_guards(void** state);
void test_dbc_load_invalid_path(void** state);
void test_dbc_lifecycle(void** state);
void test_dbc_decode_no_match(void** state);
void test_dbc_find_id_invalid_args(void** state);

#endif /* TEST_DBC_H */
