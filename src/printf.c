/** @file printf.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdarg.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
static HANDLE console = NULL;
static WORD   default_attr;
#endif /* _WIN32 */

#include "SDL.h"
#include "printf.h"

#ifdef _WIN32
static int printf_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (NULL != console)
    {
        return 0;
    }

    console = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((INVALID_HANDLE_VALUE == console) || (NULL == console))
    {
        return 1;
    }

    if (0 == GetConsoleScreenBufferInfo(console, &info))
    {
        return 2;
    }

    default_attr = info.wAttributes;
    return 0;
}
#endif /* _WIN32 */

void c_printf(const color_t fg, const color_t bg, const char* format, ...)
{
    char    buffer[1024];
    va_list varg;

#ifdef _WIN32
    WORD attr = 0;

    if (INVALID_HANDLE_VALUE == console)
    {
        console = NULL;
    }

    if (NULL == console)
    {
        if (0 != printf_init())
        {
            return;
        }
    }

    switch(fg)
    {
        default:
            attr |= default_attr;
            break;
        case DARK_BLACK:
        case LIGHT_BLACK:
            break;
        case DARK_BLUE:
        case LIGHT_BLUE:
            attr |= FOREGROUND_BLUE;
            break;
        case DARK_GREEN:
        case LIGHT_GREEN:
            attr |= FOREGROUND_GREEN;
            break;
        case DARK_CYAN:
        case LIGHT_CYAN:
            attr |= (FOREGROUND_BLUE | FOREGROUND_GREEN);
            break;
        case DARK_RED:
        case LIGHT_RED:
            attr |= FOREGROUND_RED;
            break;
        case DARK_MAGENTA:
        case LIGHT_MAGENTA:
            attr |= (FOREGROUND_RED | FOREGROUND_BLUE);
            break;
        case DARK_YELLOW:
        case LIGHT_YELLOW:
            attr |= (FOREGROUND_RED | FOREGROUND_GREEN);
            break;
        case DARK_WHITE:
        case LIGHT_WHITE:
            attr |= (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            break;
    }

    if (fg >= LIGHT_BLACK)
    {
        attr |= FOREGROUND_INTENSITY;
    }

    switch(bg)
    {
        default:
            attr |= default_attr;
            break;
        case DARK_BLACK:
        case LIGHT_BLACK:
            break;
        case DARK_BLUE:
        case LIGHT_BLUE:
            attr |= BACKGROUND_BLUE;
            break;
        case DARK_GREEN:
        case LIGHT_GREEN:
            attr |= BACKGROUND_GREEN;
            break;
        case DARK_CYAN:
        case LIGHT_CYAN:
            attr |= (BACKGROUND_BLUE | BACKGROUND_GREEN);
            break;
        case DARK_RED:
        case LIGHT_RED:
            attr |= BACKGROUND_RED;
            break;
        case DARK_MAGENTA:
        case LIGHT_MAGENTA:
            attr |= (BACKGROUND_RED | BACKGROUND_BLUE);
            break;
        case DARK_YELLOW:
        case LIGHT_YELLOW:
            attr |= (BACKGROUND_RED | BACKGROUND_GREEN);
            break;
        case DARK_WHITE:
        case LIGHT_WHITE:
            attr |= (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
            break;
    }

    if (bg >= LIGHT_BLACK)
    {
        attr |= BACKGROUND_INTENSITY;
    }

#elif defined __linux__
    switch(fg)
    {
        default:
            break;
        case DARK_BLACK:
            printf("\e[0;30m");
            break;
        case DARK_BLUE:
            printf("\e[0;34m");
            break;
        case DARK_GREEN:
            printf("\e[0;32m");
            break;
        case DARK_CYAN:
            printf("\e[0;36m");
            break;
        case DARK_RED:
            printf("\e[0;31m");
            break;
        case DARK_MAGENTA:
            printf("\e[0;35m");
            break;
        case DARK_YELLOW:
            printf("\e[0;33m");
            break;
        case DARK_WHITE:
            printf("\e[0;37m");
            break;
        case LIGHT_BLACK:
            printf("\e[0;90m");
            break;
        case LIGHT_BLUE:
            printf("\e[0;94m");
            break;
        case LIGHT_GREEN:
            printf("\e[0;92m");
            break;
        case LIGHT_CYAN:
            printf("\e[0;96m");
            break;
        case LIGHT_RED:
            printf("\e[0;91m");
            break;
        case LIGHT_MAGENTA:
            printf("\e[0;95m");
            break;
        case LIGHT_YELLOW:
            printf("\e[0;93m");
            break;
        case LIGHT_WHITE:
            printf("\e[0;97m");
            break;
    }

    switch(bg)
    {
        default:
            break;
        case DARK_BLACK:
            printf("\e[40m");
            break;
        case DARK_BLUE:
            printf("\e[44m");
            break;
        case DARK_GREEN:
            printf("\e[42m");
            break;
        case DARK_CYAN:
            printf("\e[46m");
            break;
        case DARK_RED:
            printf("\e[41m");
            break;
        case DARK_MAGENTA:
            printf("\e[45m");
            break;
        case DARK_YELLOW:
            printf("\e[43m");
            break;
        case DARK_WHITE:
            printf("\e[47m");
            break;
        case LIGHT_BLACK:
            printf("\e[0;100m");
            break;
        case LIGHT_BLUE:
            printf("\e[0;104m");
            break;
        case LIGHT_GREEN:
            printf("\e[0;102m");
            break;
        case LIGHT_CYAN:
            printf("\e[0;106m");
            break;
        case LIGHT_RED:
            printf("\e[0;101m");
            break;
        case LIGHT_MAGENTA:
            printf("\e[0;105m");
            break;
        case LIGHT_YELLOW:
            printf("\e[0;103m");
            break;
        case LIGHT_WHITE:
            printf("\e[0;107m");
            break;
    }
#endif /* _WIN32 */

   va_start(varg, format);
   SDL_vsnprintf(buffer, 1024, format, varg);
   va_end(varg);

#ifdef _WIN32
   SetConsoleTextAttribute(console, attr);
#endif

   printf("%s", buffer);

#ifdef _WIN32
   SetConsoleTextAttribute(console, default_attr);
#elif defined __linux__
   printf("\e[0m");
#endif
}
