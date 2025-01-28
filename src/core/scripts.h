/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "core.h"

bool_t   has_valid_extension(const char* filename);
void     scripts_init(core_t* core);
void     scripts_deinit(core_t* core);
status_t list_scripts(void);
void     print_heading(const char* heading);
void     run_script(char* name, core_t* core);
status_t run_script_ex(char* name, core_t* core);

#endif /* SCRIPTS_H */
