/** @file scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "dirent.h"
#include "core.h"
#include "scripts.h"

void scripts_init(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    core->L = luaL_newstate();
    luaL_openlibs(core->L);

    lua_pushcfunction(core->L, lua_delay_ms);
    lua_setglobal(core->L, "delay_ms");
}

void scripts_deinit(core_t* core)
{
    if (NULL == core)
    {
        return;
    }

    lua_close(core->L);
}

void list_scripts(void)
{
    DIR *dir;

    dir = opendir("./scripts");
    if (NULL != dir)
    {
        struct dirent *ent;
        while (NULL != (ent = readdir (dir)))
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
        closedir (dir);
    }
}

void run_script(const char* name, core_t* core)
{
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
        SDL_LogWarn(0, "Could not load script '%s'", name);
    }
}

int lua_delay_ms(lua_State* L)
{
    Uint32 delay_in_ms = (Uint32)luaL_checkinteger(L, 1);

    SDL_Delay(delay_in_ms);
    return 1;
}
