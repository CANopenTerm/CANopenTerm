/** @file scripts_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "os.h"

const uint8 max_script_search_paths = 3;

const char* script_search_path[] = {
    ".",
    "./scripts",
    "/usr/share/CANopenTerm/scripts"};
