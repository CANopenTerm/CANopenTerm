/** @file test_codb.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_CODB_H
#define TEST_CODB_H

/* CODB functionality tests */
void test_codb2json(void** state);
void test_codb_loaded_state(void** state);

/* NOTE: codb_init() and codb_deinit() initialize/cleanup global CODB state */
/* NOTE: codb_desc_lookup() and codb_info_lookup() require loaded CODB data */
/* NOTE: codb_get_ds301_profile() and codb_get_profile() return global profiles */
/* NOTE: is_ds301_loaded() and is_codb_loaded() check global state */
/* NOTE: list_codb(), load_codb(), load_codb_ex(), unload_codb() manipulate global CODB state */
/* All database operations are validated through integration tests with actual CODB files */

#endif /* TEST_CODB_H */
