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

extern void pdo_print_result(uint16 can_id, uint32 event_time_ms, uint64 data, bool_t was_successful, const char *comment);

static void c_pdo_add(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_pdo_del(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_pdo_functions[] =
{
    { c_pdo_add, "int pdo_add(int can_id, unsigned int event_time_ms, int length, char* data, int show_output, char* comment);" },
    { c_pdo_del, "int pdo_del(int can_id, int show_output, char* comment);" },
    { NULL, NULL }
};

void picoc_pdo_init(core_t* core)
{
    IncludeRegister(&core->P, "pdo.h", &setup, &picoc_pdo_functions[0], defs);
}

static void c_pdo_add(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    int         can_id          = param[0]->Val->Integer;
    uint32      event_time_ms   = param[1]->Val->UnsignedInteger;
    int         length          = param[2]->Val->Integer;
    uint64      data            = 0;
    bool_t      show_output     = (bool_t)param[4]->Val->Integer;
    const char* comment         = (const char*)param[5]->Val->Pointer;
    bool_t      was_successful;
    disp_mode_t disp_mode       = SILENT;

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    os_memcpy(&data, param[3]->Val->Pointer, sizeof(uint64));
    data = os_swap_64(data);

    was_successful = pdo_add(can_id, event_time_ms, length, data, disp_mode);

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, event_time_ms, data, was_successful, comment);
    }

    return_value->Val->Integer = (int)was_successful;
}

static void c_pdo_del(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    int         can_id      = param[0]->Val->Integer;
    bool_t      show_output = (bool_t)param[1]->Val->Integer;
    const char* comment     = (const char*)param[2]->Val->Pointer;
    disp_mode_t disp_mode   = SILENT;

    if (IS_TRUE == show_output)
    {
        pdo_print_result(can_id, 0, 0, IS_TRUE, comment);
    }

    return_value->Val->Integer = (int)pdo_del(can_id, disp_mode);
}

static void setup(Picoc* P)
{
    (void)P;
}
