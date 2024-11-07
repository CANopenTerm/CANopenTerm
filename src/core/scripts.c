/** @file scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "os.h"
#include "scripts.h"

char*  get_script_description(const char* script_path);
bool_t has_valid_extension(const char* filename);
void   strip_lua_extension(char* filename);

void scripts_init(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->L = luaL_newstate();

    if (NULL != core->L)
    {
        luaL_openlibs(core->L);
    }
}

void scripts_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    if (NULL != core->L)
    {
        lua_close(core->L);
    }
}

char* get_script_description(const char* script_path)
{
    static  char description[256] = { 0 };
    FILE_t* file = os_fopen(script_path, "r");

    if (NULL == file)
    {
        return NULL;
    }

    if (os_fgets(description, sizeof(description), file) != NULL)
    {
        char* desc_ptr;

        description[os_strcspn(description, "\r\n")] = '\0';

        desc_ptr = description;

        while ((*desc_ptr == ' ') || (*desc_ptr == '\t'))
        {
            desc_ptr++;
        }

        if (os_strncmp(desc_ptr, "//", 2) == 0)
        {
            os_memmove(desc_ptr, desc_ptr + 2, os_strlen(desc_ptr) - 1);
        }
        else if (os_strncmp(desc_ptr, "/*", 2) == 0)
        {
            os_memmove(desc_ptr, desc_ptr + 2, os_strlen(desc_ptr) - 1);
        }
        else if (os_strncmp(desc_ptr, "--[[", 4) == 0)
        {
            os_memmove(desc_ptr, desc_ptr + 4, os_strlen(desc_ptr) - 3);
        }
        else if (os_strncmp(desc_ptr, "--", 2) == 0)
        {
            os_memmove(desc_ptr, desc_ptr + 2, os_strlen(desc_ptr) - 1);
        }

        while ((*desc_ptr == ' ') || (*desc_ptr == '\t'))
        {
            desc_ptr++;
        }

        os_fclose(file);
        return os_strdup(desc_ptr);
    }

    os_fclose(file);
    return NULL;
}

bool_t has_valid_extension(const char* filename)
{
    const char* dot = os_strrchr(filename, '.');

    if (0 == (dot && os_strcmp(dot, ".c")))
    {
        return IS_TRUE;
    }
    else if (0 == (dot && os_strcmp(dot, ".lua")))
    {
        return IS_TRUE;
    }
    else
    {
        return IS_FALSE;
    }
}

void print_heading(const char* heading)
{
    if (NULL == heading)
    {
        return;
    }

    os_print(LIGHT_YELLOW, "\n%s\n", heading);
    os_print(LIGHT_CYAN, "Command  NodeID  Index   SubIndex  Length  Status  Comment                           Data\n");
}

size_t safe_strcpy(char* dest, const char* src, size_t size)
{
    size_t src_len = os_strlen(src);

    if (size > 0)
    {
        size_t copy_len = (src_len >= size) ? (size - 1) : src_len;
        os_memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }

    return src_len;
}

bool_t script_already_listed(char** listed_scripts, int count, const char* script_name)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (0 == os_strcmp(listed_scripts[i], script_name))
        {
            return IS_TRUE;
        }
    }

    return IS_FALSE;
}

void strip_lua_extension(char* filename)
{
    char* dot = os_strrchr(filename, '.');

    if (dot && dot != filename && os_strcmp(dot, ".lua") == 0)
    {
        *dot = '\0';
    }
}
