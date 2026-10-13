/** @file ctt.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CTT_H
#define CTT_H

#include "codb.h"
#include "core.h"
#include "os.h"

typedef struct eds_entry
{
    uint16 Index;
    uint8 SubIndex;

    char ParameterName[242]; /* max. 241 characters, leading \0. */
    uint8 ObjectType;
    uint16 DataType;
    uint32 LowLimit;
    uint32 HighLimit;
    acc_type_t AccessType;
    uint32 DefaultValue;
    bool PDOMapping;

} eds_entry_t;

typedef struct eds
{
    uint16 num_entries;
    eds_entry_t* entries;

} eds_t;

status_t cct_run_test(uint32 node_id, const char* report_file, const char* eds_file);

#endif /* CTT_H */
