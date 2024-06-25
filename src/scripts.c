/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef _WIN32
#include "conio.h"
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#ifdef __linux__
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

static const char* script_dirs[] = {
    "./scripts",
    "/usr/share/CANopenTerm/scripts",
};

static void set_nonblocking(int fd, int nonblocking);
static void set_terminal_raw_mode(struct termios* orig_termios);
static void reset_terminal_mode(struct termios* orig_termios);
#endif

#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "scripts.h"
#include "table.h"

static char*  get_script_description(const char* script_path);
static bool_t has_lua_extension(const char* filename);
static size_t safe_strcpy(char* dest, const char* src, size_t size);
static void   strip_extension(char* filename);
static bool_t script_already_listed(char** listed_scripts, int count, const char* script_name);

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

static char* get_script_description(const char* script_path)
{
    static char description[256] = { 0 };
    FILE*       file             = fopen(script_path, "r");

    if (NULL == file)
    {
        return NULL;
    }

    if (fgets(description, sizeof(description), file) != NULL)
    {
        char* desc_ptr;

        description[strcspn(description, "\r\n")] = '\0';

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

        fclose(file);
        return os_strdup(desc_ptr);
    }

    fclose(file);
    return NULL;
}

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

#ifdef __linux__
    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        dir = opendir(script_dirs[i]);
        if (NULL != dir)
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
#else
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
#endif

    for (i = 0; i < listed_count; i++)
    {
        os_free(listed_scripts[i]);
    }

    table_print_footer(&table);
}

void run_script(const char* name, core_t* core)
{
    bool_t   has_extension = os_strchr(name, '.') != NULL;
#ifdef __linux__
    int      i;
    char     script_path[256] = { 0 };
    bool_t   script_found = IS_FALSE;
#else
    char     script_path[64] = { 0 };
#endif

    if (NULL == core)
    {
        return;
    }

#ifdef __linux__
    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        os_snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], name);
        if (LUA_OK == luaL_dofile(core->L, script_path))
        {
            lua_pop(core->L, lua_gettop(core->L));
            return;
        }
        else
        {
            if (IS_FALSE == has_extension)
            {
                os_snprintf(script_path, sizeof(script_path), "%s/%s.lua", script_dirs[i], name);
                if (LUA_OK == luaL_dofile(core->L, script_path))
                {
                    lua_pop(core->L, lua_gettop(core->L));
                    return;
                }
            }
            if (IS_FALSE == script_found && luaL_loadfile(core->L, script_path) != LUA_ERRFILE)
            {
                script_found = IS_TRUE;
            }
        }
    }

    if (IS_FALSE == script_found)
    {
        os_log(LOG_WARNING, "Script '%s' not found in any directory", name);
    }
    else
    {
        os_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
    }
#else

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
#endif
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

        os_log(LIGHT_BLACK, "Delay ");
        os_log(DEFAULT_COLOR, "   -       -       -         -       -       ");

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        os_log(DARK_MAGENTA, "%s ", buffer);
        os_log(DEFAULT_COLOR, "%ums\n", delay_in_ms);
    }

    os_delay(delay_in_ms);
    return 1;
}

int lua_key_is_hit(lua_State * L)
{
#ifdef __linux__
    struct termios orig_termios;
    char buffer = 0;
    int n;
#endif

#ifdef _WIN32
    if (0 != kbhit())
    {
        char key = getch();
        (void)key;

        lua_pushboolean(L, 1);
    }
#elif defined __linux__

    set_terminal_raw_mode(&orig_termios);
    set_nonblocking(STDIN_FILENO, 1);

    n = read(STDIN_FILENO, &buffer, 1);

    reset_terminal_mode(&orig_termios);
    set_nonblocking(STDIN_FILENO, 0);

    if (n > 0)
    {
        lua_pushboolean(L, 1);
    }
#endif
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
}

#ifdef __linux__
static void set_nonblocking(int fd, int nonblocking)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (nonblocking)
    {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    else
    {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
}

static void set_terminal_raw_mode(struct termios* orig_termios)
{
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, orig_termios);
    new_termios = *orig_termios;
    cfmakeraw(&new_termios);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

static void reset_terminal_mode(struct termios* orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSANOW, orig_termios);
}
#endif

int lua_print_heading(lua_State* L)
{
    const char* heading = lua_tostring(L, 1);

    os_log(LIGHT_YELLOW, "\n%s\n", heading);
    os_log(LIGHT_CYAN, "Command  NodeID  Index   SubIndex  Length  Status  Comment                           Data\n");

    return 0;
}

static bool_t has_lua_extension(const char* filename)
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

static void strip_extension(char* filename)
{
    char* dot = os_strrchr(filename, '.');

    if (dot && dot != filename)
    {
        *dot = '\0';
    }
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
