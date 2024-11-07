/** @file eds.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <string.h>
#include <dirent.h>
#include "eds.h"
#include "ini.h"
#include "table.h"

static status_t run_conformance_test(const char* eds_file);

void list_eds(void)
{
    status_t status;
    table_t  table = { DARK_CYAN, DARK_WHITE, 3, 25, 1 };
    DIR_t*   d;
    int      file_no = 1;

    struct dirent_t* dir;

    status = table_init(&table, 1024);
    if (ALL_OK != status)
    {
        return;
    }

    d = os_opendir("eds");
    if (d)
    {
        table_print_header(&table);
        table_print_row("No.", "File name", "-", &table);
        table_print_divider(&table);

        while ((dir = os_readdir(d)) != NULL)
        {
            if (os_strstr(dir->d_name, ".eds") != NULL)
            {
                char file_no_str[4];
                os_snprintf(file_no_str, 4, "%3d", file_no);

                table_print_row(file_no_str, dir->d_name, "-", &table);
                file_no++;
            }
        }
        os_closedir(d);
    }
    else
    {
        os_log(LOG_WARNING, "Could not open eds directory.");
    }

    table_print_footer(&table);
    table_flush(&table);
}

status_t validate_eds(uint32 file_no, core_t* core)
{
    status_t status = ALL_OK;
    DIR_t*   d;
    int      found_file_no = 1;

    struct dirent_t* dir;

    d = os_opendir("eds");
    if (d)
    {
        while ((dir = os_readdir(d)) != NULL)
        {
            if (os_strstr(dir->d_name, ".eds") != NULL)
            {
                if (file_no == found_file_no)
                {
                    status = run_conformance_test(dir->d_name);
                    break;
                }
                found_file_no++;
            }
        }
    }
    else
    {
        os_log(LOG_WARNING, "Could not open eds directory.");
    }

    return status;
}

static status_t run_conformance_test(const char* eds_file)
{
    status_t status = ALL_OK;
    eds_t    eds;

    os_log(LOG_INFO, "Running conformance test for %s...", eds_file);

    /* Parse EDS file. */



    return status;
}
