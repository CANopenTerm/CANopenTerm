/** @file test_common.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

void test_list_file_type_invalid_dir(void** state);
void test_list_file_type_valid_dir(void** state);
void test_list_file_type_with_active_no(void** state);
void test_list_file_type_empty_extension(void** state);
void test_list_file_type_no_matching_files(void** state);

#endif /* TEST_COMMON_H */
