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

static void c_dbc_decode(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_dbc_find_id_by_name(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void c_dbc_load(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_dbc_functions[] =
{
    { c_dbc_decode,          "char* dbc_decode(int can_id, char* data);" },
    { c_dbc_find_id_by_name, "int   dbc_find_id_by_name(int* can_id, char* search);" },
    { c_dbc_load,            "int   dbc_load(char* filename);" },
    { NULL,                  NULL }
};

void picoc_dbc_init(core_t* core)
{
    IncludeRegister(&core->P, "dbc.h", &setup, &picoc_dbc_functions[0], defs);
}

static void c_dbc_decode(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    uint64 data = 0;

    if (NULL == param[1]->Val->Pointer)
    {
        return_value->Val->Pointer = NULL;
        return;
    }

    os_memcpy(&data, param[1]->Val->Pointer, sizeof(uint64));
    data = os_swap_64(data);

    return_value->Val->Pointer = (void*)dbc_decode(param[0]->Val->Integer, data);
}

static void c_dbc_find_id_by_name(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    status_t status;

    status = dbc_find_id_by_name(param[0]->Val->Pointer, (char*)param[1]->Val->Pointer);

    if (ALL_OK == status)
    {
        return_value->Val->Integer = 1;
    }
    else
    {
        return_value->Val->Integer = 0;
    }
}

static void c_dbc_load(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    status_t status;

    dbc_unload();

    status = dbc_load(param[0]->Val->Pointer);
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
