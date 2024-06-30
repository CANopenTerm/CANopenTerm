/** @file dbc.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "dbc.h"
#include "os.h"

static void   parse_message_line(char *line, message_t *current_message);
static void   parse_signal_line(char *line, message_t *current_message);
static bool_t starts_with(const char *str, const char *prefix);
static char*  trim_whitespace(char *str);

dbc_t* dbc_load(const char *filename)
{
    dbc_t*     dbc;
    char       line[1024]      = { 0 };
    FILE*      file            = os_fopen(filename, "r");
    message_t* current_message = NULL;

    if (NULL == file)
    {
        return NULL;
    }

    dbc = os_calloc(1, sizeof(dbc_t));
    if (NULL == dbc)
    {
        os_fclose(file);
        return NULL;
    }

    current_message = &dbc->messages[0];

    while (os_fgets(line, sizeof(line), file) != NULL)
    {
        char *trimmed_line = trim_whitespace(line);

        if (starts_with(trimmed_line, "BO_ "))
        {
            if (current_message != NULL)
            {
                parse_message_line(trimmed_line, current_message);
            }
        }
        else if (starts_with(trimmed_line, "SG_ "))
        {
            if (current_message != NULL)
            {
                parse_signal_line(trimmed_line, current_message);
            }
        }
    }

    os_fclose(file);

    return dbc;
}

static void parse_message_line(char *line, message_t *current_message)
{
    os_printf("message: %s\n", line);
}

static void parse_signal_line(char *line, message_t *current_message)
{
    os_printf("signal: %s\n", line);
}

static bool_t starts_with(const char *str, const char *prefix)
{
    bool_t status;
    size_t len_prefix = os_strlen(prefix);
    size_t len_str    = os_strlen(str);

    if (len_str < len_prefix)
    {
        status = IS_FALSE;
    }
    else
    {
        status = (bool_t)(os_strncmp(prefix, str, len_prefix) == 0);
    }

    return status;
}

static char* trim_whitespace(char *str)
{
    char *end;

    while (os_isspace((unsigned char)*str))
    {
        str++;
    }

    if (0  == *str)
    {
        return str;
    }

    end = str + strlen(str) - 1;
    while ((end > str) && (os_isspace((unsigned char)*end)))
    {
        end--;
    }

    *(end + 1) = 0;

    return str;
}
