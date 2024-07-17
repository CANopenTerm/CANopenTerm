/** @file picoc_nmt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "interpreter.h"
#include "nmt.h"
#include "os.h"
#include "picoc_nmt.h"

static const char defs[] = "    \
typedef enum {                  \
    NMT_OPERATIONAL     = 0x01, \
    NMT_STOP            = 0x02, \
    NMT_PRE_OPERATIONAL = 0x80, \
    NMT_RESET_NODE      = 0x81, \
    NMT_RESET_COMM      = 0x82  \
} nmt_command_t;";

static void c_nmt_send_command(struct ParseState* parser, struct Value* return_value, struct Value** param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_nmt_functions[] =
{
    {c_nmt_send_command, "int nmt_send_command(int node_id, nmt_command_t command, int show_output, char* comment);"},
    {NULL,               NULL}
};

void picoc_nmt_init(core_t* core)
{
    IncludeRegister(&core->P, "nmt.h", &setup, &picoc_nmt_functions[0], defs);
}

static void c_nmt_send_command(struct ParseState* parser, struct Value* return_value, struct Value** param, int args)
{
    status_t    status;
    disp_mode_t disp_mode = SILENT;

    if (param[2]->Val->Integer > 0)
    {
        disp_mode = SCRIPT_MODE;
    }

    status = nmt_send_command(param[0]->Val->Integer, (nmt_command_t)param[1]->Val->Integer, disp_mode, (char*)param[3]->Val->Pointer);
    if (ALL_OK == status)
    {
        return_value->Val->Integer = 0;
    }
    else
    {
        return_value->Val->Integer = 1;
    }
}

static void setup(Picoc* P)
{
    (void)P;
}
