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

static eds_t eds;

static int parse_eds(void* user, const char* section, const char* name, const char* value);
static     status_t run_conformance_test(const char* eds_file);

void list_eds(void)
{
    status_t status;
    table_t  table = { DARK_CYAN, DARK_WHITE, 3, 25, 1 };
    DIR_t* d;
    int      file_no = 1; // Move file_no declaration here

    status = table_init(&table, 1024);
    if (ALL_OK != status)
    {
        return;
    }

    d = os_opendir("eds");
    if (d)
    {
        struct dirent_t* dir;

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
                file_no++; // Increment file_no for each file
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
    DIR_t* d;
    int      found_file_no = 1; // Move found_file_no declaration here

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
                    char eds_path[50];
                    os_snprintf(eds_path, 50, "eds/%s", dir->d_name);

                    status = run_conformance_test(eds_path);
                    break;
                }
                found_file_no++; // Increment found_file_no for each file
            }
        }
        os_closedir(d);
    }
    else
    {
        os_log(LOG_WARNING, "Could not open eds directory.");
    }

    return status;
}

static int parse_eds(void* user, const char* section, const char* name, const char* value)
{
    static char prev_section[50] = "";
    size_t len;

    /*
    if (0 != os_strcmp(section, prev_section))
    {
        os_strlcpy(prev_section, section, sizeof(prev_section));
        prev_section[sizeof(prev_section) - 1] = '\0';
        os_log(LOG_INFO, "Section: %s", section);
    }
    */

    len = os_strlen(section);
    if (len > 7 &&
        os_isxdigit(section[0]) &&
        os_isxdigit(section[1]) &&
        os_isxdigit(section[2]) &&
        os_isxdigit(section[3]) &&
        section[4] == 's'       &&
        section[5] == 'u'       &&
        section[6] == 'b'       &&
        os_isxdigit(section[7]) &&
        (len == 8 || (len == 9 && os_isxdigit(section[8]))) )
    {
        char index[5]     = { 0 };
        char sub_index[3] = { 0 };

        os_strlcpy(index, section, 5);
        os_strlcpy(sub_index, section + 7, 3);

        /* Todo: add new EDS entry once (!). */



        if (0 == os_strcmp(name, "ParameterName"))
        {

        }
        else if (0 == os_strcmp(name, "ObjectType"))
        {

        }
        else if (0 == os_strcmp(name, "DataType"))
        {

        }
        else if (0 == os_strcmp(name, "LowLimit"))
        {

        }
        else if (0 == os_strcmp(name, "HighLimit"))
        {

        }
        else if (0 == os_strcmp(name, "AccessType"))
        {

        }
        else if (0 == os_strcmp(name, "DefaultValue"))
        {

        }
        else if (0 == os_strcmp(name, "PDOMapping"))
        {

        }
    }

    return 1;
}

static status_t run_conformance_test(const char* eds_path)
{
    status_t status = ALL_OK;
    int      error;

    os_log(LOG_INFO, "Running conformance test for %s...", eds_path);

    /* Parse EDS file. */
    error = ini_parse(eds_path, parse_eds, &eds_path);
    if (error < 0)
    {
        os_log(LOG_ERROR, "Can't load '%s' (%d).", eds_path, error);
        status = EDS_PARSE_ERROR;
    }

    return status;
}
