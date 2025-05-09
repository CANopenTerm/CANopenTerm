/** @file common.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <dirent.h>

#include "core.h"
#include "os.h"
#include "table.h"

void list_file_type(const char* dir, const char* ext, uint32 active_no)
{
    const char* data_path = os_find_data_path();
    char file_path[512] = {0};
    DIR_t* d;
    table_t table = {DARK_CYAN, DARK_WHITE, 3, 25, 1};
    status_t status;
    uint32 status_width = 1;

    os_snprintf(file_path, sizeof(file_path), "%s/%s", data_path, dir);

    if (active_no > 0)
    {
        status_width = 6;
    }

    table.column_c_width = status_width;
    status = table_init(&table, 1024);

    if (ALL_OK != status)
    {
        return;
    }

    d = os_opendir(file_path);
    if (d)
    {
        struct dirent_t* dir;
        uint32 file_no = 1;

        table_print_header(&table);
        if (0 == active_no)
        {
            table_print_row("No.", "File name", "-", &table);
        }
        else
        {
            table_print_row("No.", "File name", "Status", &table);
        }
        table_print_divider(&table);

        while ((dir = os_readdir(d)) != NULL)
        {
            char extension[5] = {0};
            os_strlcpy(&extension[1], ext, 4);
            extension[0] = '.';

            if (os_strstr(dir->d_name, extension) != NULL)
            {
                char file_no_str[4] = {0};

                os_snprintf(file_no_str, 4, "%3u", file_no);

                if ((active_no > 0) && (active_no == file_no))
                {
                    table_print_row(file_no_str, dir->d_name, "Active", &table);
                }
                else if (0 == active_no)
                {
                    table_print_row(file_no_str, dir->d_name, "-", &table);
                }
                else
                {
                    table_print_row(file_no_str, dir->d_name, " ", &table);
                }
                file_no++;
            }
        }
        os_closedir(d);
    }
    else
    {
        os_log(LOG_WARNING, "Could not open %s directory.", file_path);
    }

    table_print_footer(&table);
    table_flush(&table);
}
