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
#include "os.h"
#include "picoc_can.h"

static can_message_t can_msg = { 0 };

static const char defs[] = " \
typedef struct can_message { \
    int  id;                 \
    int  length;             \
    char data[0xff];         \
    long timestamp_us;       \
    int  is_extended;        \
} can_message_t;";

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_can_write(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_can_functions[] =
{
    { c_can_read,  "can_message_t* can_read(void);" },
    { c_can_write, "int can_write(can_message_t* message, int show_output, char* comment);" },
    { NULL,        NULL }
};

void picoc_can_init(core_t* core)
{
    IncludeRegister(&core->P, "can.h", &setup, &picoc_can_functions[0], defs);
}

static void c_can_read(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    uint32 status;

    status = can_read(&can_msg);

    if (ALL_OK == status)
    {
        return_value->Val->Pointer = (void*)&can_msg;
    }
    else
    {
        return_value->Val->Pointer = NULL;
    }
}

static void c_can_write(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    uint32      status;
    disp_mode_t disp_mode = SILENT;

    if (param[1]->Val->Integer > 0)
    {
        disp_mode = SCRIPT_MODE;
    }

    status = can_write((can_message_t*)param[0]->Val->Pointer, disp_mode, (char*)param[2]->Val->Pointer);

    if (ALL_OK == status)
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
