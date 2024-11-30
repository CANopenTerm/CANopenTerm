/** @file common.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <dirent.h>
#include "core.h"
#include "table.h"
#include "os.h"

void list_file_type(const char* dir, const char* ext)
{
    DIR_t*   d      = os_opendir(dir);
    table_t  table  = { DARK_CYAN, DARK_WHITE, 3, 25, 1 };
    status_t status = table_init(&table, 1024);

    if (ALL_OK != status)
    {
        return;
    }

    if (d)
    {
        struct dirent_t* dir;
        int    file_no = 1;

        table_print_header(&table);
        table_print_row("No.", "File name", "-", &table);
        table_print_divider(&table);

        while ((dir = os_readdir(d)) != NULL)
        {
            char extension[5] = { 0 };
            os_strlcpy(&extension[1], ext, 4);
            extension[0] = '.';

            if (os_strstr(dir->d_name, extension) != NULL)
            {
                char file_no_str[4] = { 0 };

                os_snprintf(file_no_str, 4, "%3d", file_no);

                table_print_row(file_no_str, dir->d_name, "-", &table);
                file_no++;
            }
        }
        os_closedir(d);
    }
    else
    {
        os_log(LOG_WARNING, "Could not open %s directory.", dir);
    }

    table_print_footer(&table);
    table_flush(&table);
}
