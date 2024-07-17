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

static const char defs[] = "";

static void setup(Picoc* P);

struct LibraryFunction picoc_misc_functions[] =
{
    {NULL, NULL}
};

void picoc_misc_init(core_t* core)
{
    IncludeRegister(&core->P, "misc.h", &setup, &picoc_misc_functions[0], defs);
}

static void setup(Picoc* P)
{
    (void)P;
}
