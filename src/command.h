/** @file command.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef COMMAND_H
#define COMMAND_H

#include "core.h"

void parse_command(char* input, core_t* core);

#endif /* COMMAND_H */
