/** @file library_can.c
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

static const char defs[] = "";

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* pc);

struct LibraryFunction library_can_functions[] =
{
    {c_can_read, "int can_read(struct can_message_t*);"},
    {NULL,       NULL}
};

void library_can_init(core_t* core)
{
    const char* can_message_t_struct = "struct can_message_t { int id; int length; char data[0xff]; long timestamp_us; int is_extended; };";
    PicocParse(&core->P, "can", can_message_t_struct, strlen(can_message_t_struct), true, false, false, false);
    IncludeRegister(&core->P, "can.h", &setup, &library_can_functions[0], defs);
}

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    return_value->Val->Integer = can_read((can_message_t*)param[0]->Val->Pointer);
}

static void setup(Picoc* pc)
{
}
