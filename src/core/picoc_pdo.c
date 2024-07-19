/** @file picoc_pdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "interpreter.h"
#include "os.h"
#include "picoc_pdo.h"
#include "pdo.h"

static const char defs[] = "";

static void setup(Picoc* P);

struct LibraryFunction picoc_pdo_functions[] =
{
    { NULL, NULL }
};

void picoc_pdo_init(core_t* core)
{
    IncludeRegister(&core->P, "pdo.h", &setup, &picoc_pdo_functions[0], defs);
}

#if 0
int lua_pdo_add(lua_State *L)
{
    int         can_id = luaL_checkinteger(L, 1);
    int         event_time_ms = luaL_checkinteger(L, 2);
    int         length = luaL_checkinteger(L, 3);
    uint64      data = lua_tointeger(L, 4);
    bool_t      show_output = lua_toboolean(L, 5);
    const char *comment = lua_tostring(L, 6);
    bool_t      success;
    disp_mode_t disp_mode = SILENT;

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    success = pdo_add(can_id, event_time_ms, length, data, disp_mode);

    if (IS_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        if (IS_TRUE == success)
        {
            os_print(LIGHT_BLACK, "PDO  ");
            os_print(DEFAULT_COLOR, "    0x%03X   -       -         -       ", can_id);
            os_print(LIGHT_GREEN, "SUCC    ");
            os_print(DARK_MAGENTA, "%s ", buffer);
            os_print(DEFAULT_COLOR, "0x%08X, %ums\n", data, event_time_ms);
        }
    }

    lua_pushboolean(L, success);

    return 1;
}

int lua_pdo_del(lua_State *L)
{
    int         can_id = luaL_checkinteger(L, 1);
    bool_t      show_output = lua_toboolean(L, 2);
    const char *comment = lua_tostring(L, 3);
    disp_mode_t disp_mode = SILENT;

    if (IS_TRUE == show_output)
    {
        int  i;
        char buffer[34] = { 0 };

        disp_mode = SCRIPT_MODE;

        if (NULL == comment)
        {
            comment = "-";
        }

        os_strlcpy(buffer, comment, 33);
        for (i = os_strlen(buffer); i < 33; ++i)
        {
            buffer[i] = ' ';
        }

        os_print(LIGHT_BLACK, "PDO  ");
        os_print(DEFAULT_COLOR, "    0x%02X   -       -         -       ", can_id);
        os_print(LIGHT_GREEN, "SUCC    ");
        os_print(DEFAULT_COLOR, "%s ", buffer);
        os_print(DEFAULT_COLOR, "Delete\n");
    }

    lua_pushboolean(L, pdo_del(can_id, disp_mode));
    return 1;
}
#endif


static void setup(Picoc* P)
{
    (void)P;
}
