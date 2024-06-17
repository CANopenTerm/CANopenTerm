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
#include <string.h>
#include <stdlib.h>

static const char* script_dirs[] = {
    "./scripts",
    "/usr/share/CANopenTerm/scripts",
};

static void set_nonblocking(int fd, int nonblocking);
static void set_terminal_raw_mode(struct termios* orig_termios);
static void reset_terminal_mode(struct termios* orig_termios);
#endif

#include "SDL.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "printf.h"
#include "scripts.h"

static SDL_bool has_lua_extension(const char* filename);
static size_t   safe_strcpy(char* dest, const char* src, size_t size);
static void     strip_extension(char* filename);
static SDL_bool script_already_listed(char** listed_scripts, int count, const char* script_name);

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


void list_scripts(void)
{
    DIR*  dir;
    char* listed_scripts[256];
    int   listed_count = 0;
    int   i;

#ifdef __linux__
    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        dir = opendir(script_dirs[i]);
        if (NULL != dir)
        {
            struct dirent* ent;
            while (NULL != (ent = readdir(dir)))
            {
                if (DT_REG == ent->d_type && has_lua_extension(ent->d_name))
                {
                    char script_name[256];
                    safe_strcpy(script_name, ent->d_name, sizeof(script_name));
                    script_name[sizeof(script_name) - 1] = '\0';
                    strip_extension(script_name);

                    if (!script_already_listed(listed_scripts, listed_count, script_name))
                    {
                        listed_scripts[listed_count++] = SDL_strdup(script_name);
                        printf("%s\n", script_name);
                    }
                }
}
            closedir(dir);
        }
    }
#else
    dir = opendir("./scripts");
    if (NULL != dir)
    {
        struct dirent* ent;
        while (NULL != (ent = readdir(dir)))
        {
            if (DT_REG == ent->d_type && has_lua_extension(ent->d_name))
            {
                char script_name[256];
                safe_strcpy(script_name, ent->d_name, sizeof(script_name));
                script_name[sizeof(script_name) - 1] = '\0';
                strip_extension(script_name);

                if (SDL_FALSE == script_already_listed(listed_scripts, listed_count, script_name))
                {
                    listed_scripts[listed_count++] = strdup(script_name);
                    printf("%s\n", script_name);
                }
            }
        }
        closedir(dir);
    }
#endif

    for (i = 0; i < listed_count; i++)
    {
        free(listed_scripts[i]);
    }
}

void run_script(const char* name, core_t* core)
{
    SDL_bool has_extension = SDL_strchr(name, '.') != NULL;
#ifdef __linux__
    int      i;
    char     script_path[256] = { 0 };
    SDL_bool script_found = SDL_FALSE;
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
        SDL_snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], name);
        if (LUA_OK == luaL_dofile(core->L, script_path))
        {
            lua_pop(core->L, lua_gettop(core->L));
            return;
        }
        else
        {
            if (SDL_FALSE == has_extension)
            {
                SDL_snprintf(script_path, sizeof(script_path), "%s/%s.lua", script_dirs[i], name);
                if (LUA_OK == luaL_dofile(core->L, script_path))
                {
                    lua_pop(core->L, lua_gettop(core->L));
                    return;
                }
            }
            if (SDL_FALSE == script_found && luaL_loadfile(core->L, script_path) != LUA_ERRFILE)
            {
                script_found = SDL_TRUE;
            }
        }
    }

    if (SDL_FALSE == script_found)
    {
        c_log(LOG_WARNING, "Script '%s' not found in any directory", name);
    }
    else
    {
        c_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
    }
#else

    SDL_snprintf(script_path, sizeof(script_path), "scripts/%s", name);
    if (LUA_OK == luaL_dofile(core->L, script_path))
    {
        lua_pop(core->L, lua_gettop(core->L));
    }
    else
    {
        if (SDL_FALSE == has_extension)
        {
            SDL_snprintf(script_path, sizeof(script_path), "scripts/%s.lua", name);
            if (LUA_OK == luaL_dofile(core->L, script_path))
            {
                lua_pop(core->L, lua_gettop(core->L));
                return;
            }
        }
        c_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
    }
#endif
}


int lua_delay_ms(lua_State * L)
{
    Uint32      delay_in_ms = (Uint32)lua_tointeger(L, 1);
    SDL_bool    show_output = lua_toboolean(L, 2);
    const char* comment     = lua_tostring(L, 3);

    if (0 == delay_in_ms)
    {
        delay_in_ms = 1000u;
    }

    if (SDL_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        c_printf(LIGHT_BLACK, "Delay ");
        c_printf(DEFAULT_COLOR, "   -       -       -         -       -       ");

        if (NULL == comment)
        {
            comment = "-";
        }

        SDL_strlcpy(buffer, comment, 33);
        for (i = SDL_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        c_printf(DARK_MAGENTA, "%s ", buffer);
        c_printf(DEFAULT_COLOR, "%ums\n", delay_in_ms);
    }

    SDL_Delay(delay_in_ms);
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

lua_print_heading(lua_State* L)
{
    const char* heading = lua_tostring(L, 1);

    c_printf(LIGHT_YELLOW, "\n%s\n", heading);
    c_printf(LIGHT_CYAN, "Command  NodeID  Index   SubIndex  Length  Status  Comment                           Data\n");

    return 0;
}

static SDL_bool has_lua_extension(const char* filename)
{
    const char* dot = SDL_strrchr(filename, '.');

    if (0 == (dot && strcmp(dot, ".lua")))
    {
        return SDL_TRUE;
    }
    else
    {
        return SDL_FALSE;
    }
}

static size_t safe_strcpy(char* dest, const char* src, size_t size)
{
    size_t src_len = SDL_strlen(src);

    if (size > 0)
    {
        size_t copy_len = (src_len >= size) ? (size - 1) : src_len;
        SDL_memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }

    return src_len;
}

static void strip_extension(char* filename)
{
    char* dot = SDL_strrchr(filename, '.');

    if (dot && dot != filename)
    {
        *dot = '\0';
    }
}

static SDL_bool script_already_listed(char** listed_scripts, int count, const char* script_name)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (0 == SDL_strcmp(listed_scripts[i], script_name))
        {
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}
