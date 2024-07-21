/** @file picoc_misc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "interpreter.h"
#include "os.h"
#include "picoc_misc.h"
#include "scripts.h"

extern void print_heading(const char* heading);

static const char defs[] = "";

static void c_delay_ms(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_key_is_hit(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_print_heading(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_misc_functions[] =
{
    { c_delay_ms,      "void delay_ms(unsigned int delay_in_ms);" },
    { c_key_is_hit,    "int key_is_hit(void);" },
    { c_print_heading, "void print_heading(char* comment);" },
    { NULL,             NULL }
};

void picoc_misc_init(core_t* core)
{
    IncludeRegister(&core->P, "misc.h", &setup, &picoc_misc_functions[0], defs);
}

static void c_delay_ms(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    os_delay(param[0]->Val->UnsignedInteger);
}

static void c_key_is_hit(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    return_value->Val->Integer = os_key_is_hit();
}

static void c_print_heading(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    print_heading((char*)param[0]->Val->Pointer);
}

static void setup(Picoc* P)
{
    (void)P;
}
