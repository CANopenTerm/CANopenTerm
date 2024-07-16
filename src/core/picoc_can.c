/** @file picoc_can.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "interpreter.h"
#include "picoc_can.h"

static const char defs[] = " \
typedef struct can_message { \
    int  id; \
    int  length; \
    char data[0xff]; \
    long timestamp_us; \
    int is_extended; \
} can_message_t;";

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_can_functions[] =
{
    {c_can_read, "int can_read(can_message_t* message);"},
    {NULL,       NULL}
};

void picoc_can_init(core_t* core)
{
    IncludeRegister(&core->P, "can.h", &setup, &picoc_can_functions[0], defs);
}

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    return_value->Val->Integer = can_read((can_message_t*)param[0]->Val->Pointer);
}

static void setup(Picoc* P)
{
    (void)P;
}
