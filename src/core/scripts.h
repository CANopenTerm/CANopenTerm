/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "core.h"

bool has_valid_extension(const char* filename);
void scripts_init(core_t* core);
void scripts_deinit(core_t* core);
void scripts_add_completions(completions_t* cenv);
status_t list_scripts(void);
void print_heading(const char* heading);
void print_result(uint8 id, uint16 index, uint8 sub_index, uint32 length, bool success, const char* comment, uint32_t data);
void run_script(char* name, core_t* core);
status_t run_script_ex(char* name, core_t* core);

#endif /* SCRIPTS_H */
