/** @file scripts_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "os.h"

const uint8 max_script_search_paths = 2;

const char* script_search_path[2] = {
    ".",
    "./scripts"};
