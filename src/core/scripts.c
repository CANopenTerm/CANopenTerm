/** @file scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "scripts.h"
#include "core.h"
#include "dirent.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "os.h"
#include "pocketpy.h"
#include "table.h"

extern const uint8 max_script_search_paths;
extern const char* script_search_path[];

static char*  get_script_description(char* script_path);
static size_t safe_strcpy(char* dest, const char* src, size_t size);
static bool_t script_already_listed(char** listed_scripts, int count, const char* script_name);
static void   strip_lua_extension(char* filename);

void scripts_init(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    py_initialize();

    core->L = luaL_newstate();

    if (NULL != core->L)
    {
        int         i;
        const char* current_path;
        char*       new_path;

        luaL_openlibs(core->L);

        lua_getglobal(core->L, "package");
        lua_getfield(core->L, -1, "path");

        current_path    = lua_tostring(core->L, -1);
        size_t path_len = os_strlen(current_path) + 1;

        for (i = 0; i < max_script_search_paths; i++)
        {
            path_len += os_strlen(script_search_path[i]) + 7;  /* Adding space for "?.lua;" */
            path_len += os_strlen(script_search_path[i]) + 12; /* Adding space for "?/init.lua;" */
        }

        new_path = (char*)os_calloc(path_len, sizeof(char));
        if (NULL == new_path)
        {
            lua_pop(core->L, 2);
            return;
        }

        os_strlcpy(new_path, current_path, path_len);

        for (i = 0; i < max_script_search_paths; i++)
        {
            os_strlcat(new_path, ";", path_len);
            os_strlcat(new_path, script_search_path[i], path_len);
            os_strlcat(new_path, "/?.lua", path_len);
            os_strlcat(new_path, ";", path_len);
            os_strlcat(new_path, script_search_path[i], path_len);
            os_strlcat(new_path, "/?/init.lua", path_len);
        }

        lua_pushstring(core->L, new_path);
        lua_setfield(core->L, -3, "path");

        os_free((void*)new_path);
        lua_pop(core->L, 2);
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

    py_finalize();
}

static char* get_script_description(char* script_path)
{
    static char description[256] = {0};
    FILE_t*     file;

    os_fix_path(script_path);
    file = os_fopen(script_path, "r");
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
        else if (os_strncmp(desc_ptr, "#", 1) == 0)
        {
            os_memmove(desc_ptr, desc_ptr + 1, os_strlen(desc_ptr));
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
    const char* dot;

    if (NULL == filename)
    {
        return IS_FALSE;
    }

    dot = os_strrchr(filename, '.');
    if (NULL == dot)
    {
        return IS_FALSE;
    }

    if ((os_strcmp(dot, ".lua") == 0 || os_strcmp(dot, ".py") == 0))
    {
        return IS_TRUE;
    }
    else
    {
        return IS_FALSE;
    }
}

status_t list_scripts(void)
{
    status_t status;
    table_t  table = {DARK_CYAN, DARK_WHITE, 3, 10, 40};

    status = table_init(&table, 1024);
    if (ALL_OK == status)
    {
        char* listed_scripts[256];
        int   listed_count = 0;
        int   i, j;

        table_print_header(&table);
        table_print_row("No.", "Identifier", "Description", &table);
        table_print_divider(&table);

        for (i = 0; i < max_script_search_paths; i++)
        {
            DIR* dir;
            char search_path[PATH_MAX] = {0};

            os_snprintf(search_path, sizeof(search_path), "%s", script_search_path[i]);
            dir = os_opendir(search_path);

            if (dir != NULL)
            {
                struct dirent* ent;
                while ((ent = readdir(dir)) != NULL)
                {
                    if (DT_REG == ent->d_type && has_valid_extension(ent->d_name))
                    {
                        char script_name[256] = {0};

                        safe_strcpy(script_name, ent->d_name, sizeof(script_name));
                        script_name[sizeof(script_name) - 1] = '\0';
                        strip_lua_extension(script_name);

                        if (IS_FALSE == script_already_listed(listed_scripts, listed_count, script_name))
                        {
                            char  buf[4]           = {0};
                            char* script_no        = os_itoa(listed_count, buf, 10);
                            char  script_path[512] = {0};
                            char* description;

                            listed_scripts[listed_count++] = os_strdup(script_name);

                            snprintf(script_path, sizeof(script_path), "%s/%s", search_path, ent->d_name);

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

        for (j = 0; j < listed_count; j++)
        {
            os_free(listed_scripts[j]);
        }

        table_print_footer(&table);
        table_flush(&table);
    }

    return status;
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

void run_script(char* name, core_t* core)
{
    status_t    status             = ALL_OK;
    const char* base               = os_strrchr(name, '/');
    char        basename[PATH_MAX] = {0};
    FILE*       file;

    if (base)
    {
        os_strlcpy(basename, base + 1, sizeof(basename) - 1);
    }
    else
    {
        os_strlcpy(basename, name, sizeof(basename) - 1);
    }

    os_fix_path(name);
    file = os_fopen(name, "r");
    if (NULL != file)
    {
        fclose(file);
        status = run_script_ex(name, core);
    }
    else
    {
        int i;

        for (i = 0; i < max_script_search_paths; i++)
        {
            char script_path[PATH_MAX] = {0};

            os_snprintf(script_path, sizeof(script_path), "%s/%s", script_search_path[i], name);

            status = run_script_ex(script_path, core);
            if (ALL_OK == status)
            {
                break;
            }
        }

        if (ALL_OK != status)
        {
            const char* user_directory        = os_get_user_directory();
            char        script_path[PATH_MAX] = {0};

            os_snprintf(script_path, sizeof(script_path), "%s/CANopenTerm/scripts/%s", user_directory, name);

            status = run_script_ex(script_path, core);
        }
    }

    if (OS_FILE_NOT_FOUND == status)
    {
        os_log(LOG_ERROR, "Script \"%s\" not found.\n", basename);
    }
}

status_t run_script_ex(char* name, core_t* core)
{
    status_t    status            = ALL_OK;
    const char* extension         = os_strrchr(name, '.');
    bool_t      has_lua_extension = extension && os_strcmp(extension, ".lua") == 0;
    bool_t      has_py_extension  = extension && os_strcmp(extension, ".py") == 0;
    char        script_path[1024] = {0};
    FILE*       file;

    if (NULL == core)
    {
        return OS_INVALID_ARGUMENT;
    }

    if (IS_TRUE == has_lua_extension)
    {
        os_snprintf(script_path, sizeof(script_path), "%s", name);

        os_fix_path(script_path);
        file = os_fopen(script_path, "r");
        if (NULL == file)
        {
            status = OS_FILE_NOT_FOUND;
        }
        else
        {
            os_fclose(file);

            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
            }
            else
            {
                os_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
                status = SCRIPT_ERROR;
            }
        }
    }
    else if (IS_TRUE == has_py_extension)
    {
        os_snprintf(script_path, sizeof(script_path), "%s", name);

        os_fix_path(script_path);
        file = os_fopen(script_path, "r");
        if (file != NULL)
        {
            os_fclose(file);

            os_fix_path(script_path);
            file = os_fopen(script_path, "rb");
            if (file == NULL)
            {
                status = OS_FILE_NOT_FOUND;
            }
            else
            {
                size_t size;
                char*  buffer;

                os_fseek(file, 0, SEEK_END);
                size = os_ftell(file);
                os_fseek(file, 0, SEEK_SET);
                buffer       = os_calloc(size + 1, sizeof(char));
                size         = os_fread(buffer, 1, size, file);
                buffer[size] = 0;

                if (IS_FALSE == py_exec(buffer, script_path, EXEC_MODE, NULL))
                {
                    py_printexc();
                    status = SCRIPT_ERROR;
                }

                os_free(buffer);
                os_fclose(file);
            }
        }
        else
        {
            status = OS_FILE_NOT_FOUND;
        }
    }
    else
    {
        os_snprintf(script_path, sizeof(script_path), "%s.lua", name);

        os_fix_path(script_path);
        file = os_fopen(script_path, "r");
        if (NULL == file)
        {
            os_snprintf(script_path, sizeof(script_path), "%s.py", name);

            os_fix_path(script_path);
            file = os_fopen(script_path, "r");
            if (NULL == file)
            {
                status = OS_FILE_NOT_FOUND;
            }
            else
            {
                size_t size;
                char*  buffer;

                os_fseek(file, 0, SEEK_END);
                size = os_ftell(file);
                os_fseek(file, 0, SEEK_SET);
                buffer       = os_calloc(size + 1, sizeof(char));
                size         = os_fread(buffer, 1, size, file);
                buffer[size] = 0;

                if (IS_FALSE == py_exec(buffer, script_path, EXEC_MODE, NULL))
                {
                    py_printexc();
                    status = SCRIPT_ERROR;
                }

                os_free(buffer);
                os_fclose(file);
            }
        }
        else
        {
            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
            }
            else
            {
                os_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
                status = SCRIPT_ERROR;
            }
        }
    }

    return status;
}

static size_t safe_strcpy(char* dest, const char* src, size_t size)
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

static bool_t script_already_listed(char** listed_scripts, int count, const char* script_name)
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

static void strip_lua_extension(char* filename)
{
    char* dot = os_strrchr(filename, '.');

    if (dot && dot != filename && os_strcmp(dot, ".lua") == 0)
    {
        *dot = '\0';
    }
}
