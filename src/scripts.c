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

static const char* script_dirs[] = { "/usr/local/share/CANopenTerm/scripts", "/usr/share/CANopenTerm/scripts", "./scripts" };

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
    DIR* dir;

#ifdef __linux__
    int i;
    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        dir = opendir(script_dirs[i]);
        if (NULL != dir)
        {
            struct dirent* ent;
            while (NULL != (ent = readdir(dir)))
            {
                switch (ent->d_type)
                {
                case DT_REG:
                    printf("%s/%s\n", script_dirs[i], ent->d_name);
                    break;
                case DT_DIR:
                case DT_LNK:
                default:
                    break;
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
            switch (ent->d_type)
            {
            case DT_REG:
                puts(ent->d_name);
                break;
            case DT_DIR:
            case DT_LNK:
            default:
                break;
            }
        }
        closedir(dir);
    }
#endif
}

void run_script(const char* name, core_t* core)
{
#ifdef __linux__
    char script_path[256] = { 0 };
    const char* script_dirs[] = { "/usr/local/share/CANopenTerm/scripts", "/usr/share/CANopenTerm/scripts", "./scripts" };
    int i;

    if (NULL == core)
    {
        return;
    }

    for (i = 0; i < sizeof(script_dirs) / sizeof(script_dirs[0]); i++)
    {
        snprintf(script_path, sizeof(script_path), "%s/%s", script_dirs[i], name);
        if (LUA_OK == luaL_dofile(core->L, script_path))
        {
            lua_pop(core->L, lua_gettop(core->L));
            return;
        }
    }

    c_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
#else
    char script_path[64] = { 0 };

    if (NULL == core)
    {
        return;
    }

    SDL_snprintf(script_path, 64, "scripts/%s", name);
    if (LUA_OK == luaL_dofile(core->L, script_path))
    {
        lua_pop(core->L, lua_gettop(core->L));
    }
    else
    {
        c_log(LOG_WARNING, "Could not run script '%s': %s", name, lua_tostring(core->L, -1));
    }
#endif
}

int lua_delay_ms(lua_State * L)
{
    Uint32      delay_in_ms = (Uint32)luaL_checkinteger(L, 1);
    SDL_bool    show_output = lua_toboolean(L, 2);
    const char* comment     = lua_tostring(L, 3);

    if (SDL_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        c_printf(DARK_CYAN, "Delay ");
        c_printf(DEFAULT_COLOR, "   -       -       -         -       -       ");

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
