/** @file picoc_test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "eds.h"
#include "interpreter.h"
#include "os.h"
#include "picoc_test_report.h"
#include "test_report.h"

static const char defs[] = " \
typedef struct test_result   \
{                            \
    int    has_passed;       \
    int    reserved_1;       \
    int    reserved_2;       \
    float  time;             \
    char*  package;          \
    char*  class_name;       \
    char*  test_name;        \
    char*  error_type;       \
    char*  error_message;    \
    char*  call_stack;       \
} test_result_t;";

static void c_test_add_result(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_test_clear_results(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_test_eds_file(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_test_generate_report(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_test_functions[] =
{
    { c_test_add_result,      "void test_add_result(test_result_t* result);" },
    { c_test_clear_results,   "void test_clear_results(void);" },
    { c_test_eds_file,        "void test_eds_file(int node_id, char* file_name);" },
    { c_test_generate_report, "int test_generate_report(char* file_name);" },
    { NULL,                   NULL }
};

void picoc_test_init(core_t* core)
{
    IncludeRegister(&core->P, "test_report.h", &setup, &picoc_test_functions[0], defs);
}

static void c_test_add_result(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    test_result_t* result_pt = (test_result_t*)param[0]->Val->Pointer;
    test_result_t  result    = { 0, 0ul, 0.f, NULL, NULL, NULL, NULL, NULL, NULL };

    result.has_passed          = result_pt->has_passed;
    result.testsuite_name_hash = result_pt->testsuite_name_hash;
    result.time                = result_pt->time;
    result.package             = result_pt->package;
    result.class_name          = result_pt->class_name;
    result.test_name           = result_pt->test_name;
    result.error_type          = result_pt->error_type;
    result.error_message       = result_pt->error_message;

    test_add_result(&result);
}

static void c_test_clear_results(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    test_clear_results();
}

static void c_test_eds_file(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    run_conformance_test((const char*)param[1]->Val->Pointer, (uint32)param[0]->Val->LongInteger, SCRIPT_MODE);
}

static void c_test_generate_report(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    if (ALL_OK == test_generate_report(param[0]->Val->Pointer))
    {
        return_value->Val->Integer = 1;
    }
    else
    {
        return_value->Val->Integer = 0;
    }
}

static void setup(Picoc* P)
{
    (void)P;
}
