/** @file codb2json.h
 *
 *  CANopen Profile data base to JSON converter.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CODB2JSON_H
#define CODB2JSON_H

#include "os.h"

int codb2json(int argc, char* argv[], bool format_output);

#endif /* CODB2JSON_H */
