/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <string.h>
#include "conio.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "os.h"
#include "scripts.h"
#include "table.h"

extern char*  get_script_description(const char* script_path);
extern bool_t has_lua_extension(const char* filename);
extern size_t safe_strcpy(char* dest, const char* src, size_t size);
extern bool_t script_already_listed(char** listed_scripts, int count, const char* script_name);
extern void   strip_extension(char* filename);

void list_scripts(void)
{
    DIR*    dir;
    char*   listed_scripts[256];
    int     listed_count = 0;
    int     i;
    table_t table = { DARK_CYAN, DARK_WHITE, 3, 10, 40 };

    table_print_header(&table);
    table_print_row("No.", "Identifier", "Description", &table);
    table_print_divider(&table);

    dir = opendir("./scripts");

    if (dir != NULL)
    {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL)
        {
            if (DT_REG == ent->d_type && has_lua_extension(ent->d_name))
            {
                char script_name[256] = { 0 };

                safe_strcpy(script_name, ent->d_name, sizeof(script_name));
                script_name[sizeof(script_name) - 1] = '\0';
                strip_extension(script_name);

                if (IS_FALSE == script_already_listed(listed_scripts, listed_count, script_name))
                {
                    char  buf[4]           = { 0 };
                    char* script_no        = os_itoa(listed_count, buf, 10);
                    char  script_path[512] = { 0 };
                    char* description;

                    listed_scripts[listed_count++] = os_strdup(script_name);

                    snprintf(script_path, sizeof(script_path), "./scripts/%s", ent->d_name);

                    description = get_script_description(script_path);
                    if (description != NULL)
                    {
                        table_print_row(script_no, script_name, description, &table);
                        os_free(description);
                    }
                    else
                    {
                        table_print_row(script_no, script_name, "-", &table);
                    }
                }
            }
        }
        closedir(dir);
    }

    for (i = 0; i < listed_count; i++)
    {
        os_free(listed_scripts[i]);
    }

    table_print_footer(&table);
}

void run_script(const char* name, core_t* core)
{
    bool_t   has_extension = os_strchr(name, '.') != NULL;
    char     script_path[64] = { 0 };

    if (NULL == core)
    {
        return;
    }

    os_snprintf(script_path, sizeof(script_path), "scripts/%s", name);
    if (LUA_OK == luaL_dofile(core->L, script_path))
    {
        lua_pop(core->L, lua_gettop(core->L));
    }
    else
    {
        if (IS_FALSE == has_extension)
        {
            os_snprintf(script_path, sizeof(script_path), "scripts/%s.lua", name);
            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
                return;
            }
        }
        os_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
    }
}

int lua_key_is_hit(lua_State * L)
{
    if (0 != kbhit())
    {
        char key = getch();
        (void)key;

        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}