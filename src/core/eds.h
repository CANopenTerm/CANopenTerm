/** @file eds.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef EDS_H
#define EDS_H

#include "codb.h"
#include "core.h"

void     list_eds(void);
status_t run_conformance_test(const char* eds_path, const char* package, uint32 node_id, disp_mode_t disp_mode);
status_t validate_eds(uint32 file_no, const char* package, uint32 node_id);

typedef struct eds_entry
{
    uint16 Index;
    uint8  SubIndex;

    char       ParameterName[242]; /* max. 241 characters, leading \0. */
    uint8      ObjectType;
    uint16     DataType;
    uint32     LowLimit;
    uint32     HighLimit;
    acc_type_t AccessType;
    uint32     DefaultValue;
    bool_t     PDOMapping;

} eds_entry_t;

typedef struct eds
{
    uint16       num_entries;
    eds_entry_t* entries;

} eds_t;

#endif /* EDS_H */
