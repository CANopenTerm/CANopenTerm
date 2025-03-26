
/** @file test_report.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_REPORT_H
#define TEST_REPORT_H

#include "os.h"

typedef struct
{
    bool has_passed;
    uint64 testsuite_name_hash;
    float time;
    const char* package;
    const char* class_name;
    const char* test_name;
    const char* error_type;
    const char* error_message;
    const char* call_stack;

} test_result_t;

status_t test_init(void);
void test_add_result(test_result_t* result);
void test_clear_results(void);
status_t test_generate_report(const char* file_name);

#endif /* TEST_REPORT_H */
