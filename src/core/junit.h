/** @file junit.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef JUNIT_H
#define JUNIT_H

#include "os.h"

typedef struct
{
    const char* package;
    const char* class_name;
    const char* error_type;
    const char* error_message;
    const char* call_stack;
    const char* test_name;
    bool_t      has_passed;
    float       time;

} junit_result_t;

status_t junit_init(void);
void     junit_add_result(junit_result_t* result);
void     junit_clear_results(void);
status_t junit_generate_report(const char* filename);

#endif /* JUNIT_H */
