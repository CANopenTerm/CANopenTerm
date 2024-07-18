/** @file picoc_pdo.c
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
#include "picoc_pdo.h"
#include "pdo.h"

static const char defs[] = "";

static void setup(Picoc* P);

struct LibraryFunction picoc_pdo_functions[] =
{
    { NULL, NULL }
};

void picoc_pdo_init(core_t* core)
{
    IncludeRegister(&core->P, "pdo.h", &setup, &picoc_pdo_functions[0], defs);
}

static void setup(Picoc* P)
{
    (void)P;
}
