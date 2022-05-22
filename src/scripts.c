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
#include "scripts.h"

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
