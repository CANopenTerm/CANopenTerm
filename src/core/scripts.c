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
bool_t has_lua_extension(const char* filename);
void   strip_extension(char* filename);

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

        lua_pushcfunction(core->L, lua_delay_ms);
        lua_setglobal(core->L, "delay_ms");

        lua_pushcfunction(core->L, lua_key_is_hit);
        lua_setglobal(core->L, "key_is_hit");

        lua_pushcfunction(core->L, lua_print_heading);
        lua_setglobal(core->L, "print_heading");
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
    static char description[256] = { 0 };
    FILE* file = os_fopen(script_path, "r");

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

        if (os_strncmp(desc_ptr, "--[[", 4) == 0)
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

bool_t has_lua_extension(const char* filename)
{
    const char* dot = os_strrchr(filename, '.');

    if (0 == (dot && strcmp(dot, ".lua")))
    {
        return IS_TRUE;
    }
    else
    {
        return IS_FALSE;
    }
}

int lua_delay_ms(lua_State * L)
{
    uint32      delay_in_ms = (uint32)lua_tointeger(L, 1);
    bool_t      show_output = lua_toboolean(L, 2);
    const char* comment     = lua_tostring(L, 3);

    if (0 == delay_in_ms)
    {
        delay_in_ms = 1000u;
    }

    if (IS_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        os_print(LIGHT_BLACK, "Delay ");
        os_print(DEFAULT_COLOR, "   -       -       -         -       -       ");

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        os_print(DARK_MAGENTA, "%s ", buffer);
        os_print(DEFAULT_COLOR, "%ums\n", delay_in_ms);
    }

    os_delay(delay_in_ms);
    return 1;
}

int lua_print_heading(lua_State* L)
{
    const char* heading = lua_tostring(L, 1);

    os_print(LIGHT_YELLOW, "\n%s\n", heading);
    os_print(LIGHT_CYAN, "Command  NodeID  Index   SubIndex  Length  Status  Comment                           Data\n");

    return 0;
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

void strip_extension(char* filename)
{
    char* dot = os_strrchr(filename, '.');

    if (dot && dot != filename)
    {
        *dot = '\0';
    }
}
