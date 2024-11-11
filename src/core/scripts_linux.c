/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <dirent.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "os.h"
#include "picoc.h"
#include "picoc_can.h"
#include "picoc_dbc.h"
#include "picoc_misc.h"
#include "picoc_nmt.h"
#include "picoc_pdo.h"
#include "picoc_sdo.h"
#include "scripts.h"
#include "table.h"

static const char* script_dirs[] = {
    "./tests",
    "./scripts",
    "/usr/share/CANopenTerm/scripts"
};

extern char*  get_script_description(const char* script_path);
extern bool_t has_valid_extension(const char* filename);
extern size_t safe_strcpy(char* dest, const char* src, size_t size);
extern bool_t script_already_listed(char** listed_scripts, int count, const char* script_name);
extern void   strip_lua_extension(char* filename);

status_t list_scripts(void)
{
    status_t status;
    table_t  table = { DARK_CYAN, DARK_WHITE, 3, 10, 40 };

    status = table_init(&table, 1024);
    if (ALL_OK == status)
    {
        char* listed_scripts[256];
        int   listed_count = 0;
        int   i;

        table_print_header(&table);
        table_print_row("No.", "Identifier", "Description", &table);
        table_print_divider(&table);

        for (i = 1; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
        {
            DIR* dir = opendir(script_dirs[i]);
            if (NULL != dir)
            {
                struct dirent* ent;

                while ((ent = readdir(dir)) != NULL)
                {
                    if (DT_REG == ent->d_type && has_valid_extension(ent->d_name))
                    {
                        char script_name[256] = { 0 };

                        safe_strcpy(script_name, ent->d_name, sizeof(script_name));
                        script_name[sizeof(script_name) - 1] = '\0';
                        strip_lua_extension(script_name);

                        if (IS_FALSE == script_already_listed(listed_scripts, listed_count, script_name))
                        {
                            char  buf[4] = { 0 };
                            char* script_no = os_itoa(listed_count, buf, 10);
                            char  script_path[512] = { 0 };
                            char* description;

                            listed_scripts[listed_count++] = os_strdup(script_name);

                            os_snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], ent->d_name);

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
        }

        for (i = 0; i < listed_count; i++)
        {
            os_free(listed_scripts[i]);
        }

        table_print_footer(&table);
        table_flush(&table);
    }
}

void run_script(const char* name, core_t* core)
{
    int         i;
    char        script_path[256] = { 0 };
    bool_t      script_found = IS_FALSE;
    const char* extension = os_strrchr(name, '.');
    bool_t      has_c_extension = extension && os_strcmp(extension, ".c")     == 0;
    bool_t      has_lua_extension = extension && os_strcmp(extension, ".lua") == 0;

    if (NULL == core)
    {
        return;
    }

    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        if (IS_TRUE == has_lua_extension)
        {
            FILE *file = os_fopen(name, "r");

            if (file != NULL)
            {
                os_snprintf(script_path, sizeof(script_path), "%s", name);
                os_fclose(file);
            }
            else
            {
                os_snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], name);
            }

            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
                return;
            }
        }
        else if (IS_TRUE == has_c_extension)
        {
            FILE* file;

            PicocInitialize(&core->P, (128000 * 4));
            PicocIncludeAllSystemHeaders(&core->P);
            picoc_can_init(core);
            picoc_dbc_init(core);
            picoc_misc_init(core);
            picoc_nmt_init(core);
            picoc_pdo_init(core);
            picoc_sdo_init(core);

            file = os_fopen(name, "r");
            if (file != NULL)
            {
                os_snprintf(script_path, sizeof(script_path), "%s", name);
                os_fclose(file);
            }
            else
            {
                os_snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], name);
            }

            file = fopen(script_path, "r");
            if (file != NULL)
            {
                fclose(file);

                if (PicocPlatformSetExitPoint(&core->P))
                {
                    PicocCleanup(&core->P);
                    return;
                }
                script_found = IS_TRUE;
            }

            if (IS_TRUE == script_found)
            {
                PicocPlatformScanFile(&core->P, script_path);
                PicocCleanup(&core->P);
                break;
            }
            else
            {
                PicocCleanup(&core->P);
                continue;
            }
        }
        else
        {
            os_snprintf(script_path, sizeof(script_path), "%s/%s.lua", script_dirs[i], name);
            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
                return;
            }
        }
    }

    if (IS_FALSE == script_found)
    {
        os_log(LOG_WARNING, "Script '%s' not found in any directory", name);
    }
    else
    {
        if (IS_FALSE == has_c_extension)
        {
            os_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
        }
    }
}
