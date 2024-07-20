/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "core.h"

void     scripts_init(core_t* core);
void     scripts_deinit(core_t* core);
status_t list_scripts(void);
void     run_script(const char* name, core_t* core);

#endif /* SCRIPTS_H */
