/** @file command.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef COMMAND_H
#define COMMAND_H

#include "core.h"
#include "os.h"

#define COMMAND_BUFFER_SIZE 1024

void parse_command(char* input, core_t* core);
void completion_callback(const char* buf, completions_t* lc);

#endif /* COMMAND_H */
