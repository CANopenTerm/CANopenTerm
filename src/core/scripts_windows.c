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

static char script_path_install[512] = {0};
static char script_path_scripts[512] = {0};
static bool paths_initialized = false;

const char* script_search_path[2] = {NULL, NULL};

void init_script_search_paths(void)
{
    if (!paths_initialized)
    {
        const char* base_path = os_find_data_path();

        os_snprintf(script_path_install, sizeof(script_path_install), "%s", base_path);
        os_snprintf(script_path_scripts, sizeof(script_path_scripts), "%s/scripts", base_path);

        script_search_path[0] = script_path_install;
        script_search_path[1] = script_path_scripts;

        paths_initialized = true;
    }
}
