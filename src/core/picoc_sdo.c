/** @file picoc_sdo.c
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
#include "picoc_sdo.h"
#include "sdo.h"

static const char defs[] = "";

static void setup(Picoc* P);

struct LibraryFunction picoc_sdo_functions[] =
{
    {NULL, NULL}
};

void picoc_sdo_init(core_t* core)
{
    IncludeRegister(&core->P, "sdo.h", &setup, &picoc_sdo_functions[0], defs);
}

static void setup(Picoc* P)
{
    (void)P;
}
