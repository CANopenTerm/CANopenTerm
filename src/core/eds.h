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

#include "core.h"

void list_eds(void);
void validate_eds(const char* name, core_t* core);

#endif /* EDS_H */
