/** @file picoc_dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "dbc.h"
#include "interpreter.h"
#include "os.h"
#include "picoc_dbc.h"

static const char defs[] = "";

static void setup(Picoc* P);

struct LibraryFunction picoc_dbc_functions[] =
{
    {NULL, NULL}
};

void picoc_dbc_init(core_t* core)
{
    IncludeRegister(&core->P, "dbc.h", &setup, &picoc_dbc_functions[0], defs);
}

static void setup(Picoc* P)
{
    (void)P;
}
