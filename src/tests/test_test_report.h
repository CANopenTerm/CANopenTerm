/** @file test_test_report.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_TEST_REPORT_H
#define TEST_TEST_REPORT_H

void test_report_init(void** state);
void test_report_clear(void** state);
void test_report_add_and_generate(void** state);
void test_report_generate_null_filename(void** state);

#endif /* TEST_TEST_REPORT_H */
