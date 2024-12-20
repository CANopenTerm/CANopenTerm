/** @file scripts_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "os.h"

const uint8 max_script_search_paths = 3;

const char* script_search_path[3] = {
    "./",
    "./scripts/",
    "C:/Program Files/CANopenTerm/scripts/"};
