/** @file os_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <conio.h>
#include <shlobj.h>
#include <windows.h>
#include "buffer.h"
#include "os.h"

static bool_t console_is_plain_mode;
static HANDLE console = NULL;
static WORD   default_attr;

os_timer_id os_add_timer(uint32 interval, os_timer_cb callback, void* param)
{
    return SDL_AddTimer(interval, callback, param);
}

status_t os_console_init(bool_t is_plain_mode)
{
    SetConsoleOutputCP(65001);
    SetConsoleTitle("CANopenTerm");

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

    default_attr          = info.wAttributes;
    console_is_plain_mode = is_plain_mode;

    return ALL_OK;
}

os_thread* os_create_thread(os_thread_func fn, const char* name, void* data)
{
    return SDL_CreateThread(fn, name, data);
}

void os_delay(uint32 delay_in_ms)
{
    SDL_Delay(delay_in_ms);
}

void os_detach_thread(os_thread* thread)
{
    SDL_DetachThread(thread);
}

const char* os_get_error(void)
{
    return SDL_GetError();
}

status_t os_get_prompt(char prompt[PROMPT_BUFFER_SIZE])
{
    status_t status = ALL_OK;

    if (NULL == fgets(prompt, PROMPT_BUFFER_SIZE - 1, stdin))
    {
        if (0 != feof(stdin))
        {
            status = 1; /* TODO: Add proper error. */
        }
    }

    return status;
}

uint64 os_get_ticks(void)
{
    return SDL_GetTicks64();
}

const char* os_get_user_directory(void)
{
    static char user_directory[MAX_PATH] = { 0 };
    if (0 == user_directory[0])
    {
        SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, user_directory);
    }

    return user_directory;
}

status_t os_init(void)
{
    status_t status = ALL_OK;

    if (0 != SDL_InitSubSystem(SDL_INIT_TIMER))
    {
        os_log(LOG_ERROR, "Unable to initialise timer sub-system: %s", os_get_error());
        status = OS_INIT_ERROR;
    }

    return status;
}

bool_t os_key_is_hit(void)
{
    if (0 != _kbhit())
    {
        char key = _getch();
        (void)key;

        return IS_TRUE;
    }
    else
    {
        return IS_FALSE;
    }
}

void os_log(const log_level_t level, const char* format, ...)
{
    char      buffer[1024];
    va_list_t varg;

    if (LOG_SUPPRESS == level)
    {
        return;
    }

    os_va_start(varg, format);
    os_vsnprintf(buffer, 1024, format, varg);
    os_va_end(varg);

    switch (level)
    {
        default:
        case LOG_DEFAULT:
            break;
        case LOG_INFO:
            os_print(DARK_WHITE, "[INFO]    ");
            break;
        case LOG_SUCCESS:
            os_print(LIGHT_GREEN, "[SUCCESS] ");
            break;
        case LOG_WARNING:
            os_print(DARK_YELLOW, "[WARNING] ");
            break;
        case LOG_ERROR:
            os_print(LIGHT_RED, "[ERROR]   ");
            break;
    }
    os_print(DARK_WHITE, "%s\r\n", buffer);
}

void os_print(const color_t color, const char* format, ...)
{
    char      buffer[1024];
    va_list_t args;
    WORD      attr = 0;

    if (INVALID_HANDLE_VALUE == console)
    {
        console = NULL;
    }

    if (NULL == console)
    {
        if (ALL_OK != os_console_init(console_is_plain_mode))
        {
            return;
        }
    }

    switch (color)
    {
        case DEFAULT_COLOR:
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

    if (color >= LIGHT_BLACK)
    {
        attr |= FOREGROUND_INTENSITY;
    }

    os_va_start(args, format);
    os_vsnprintf(buffer, 1024, format, args);
    os_va_end(args);

    if (IS_TRUE == use_buffer())
    {
        buffer_write(buffer);
    }
    else
    {
        if (NULL != console && IS_FALSE == console_is_plain_mode)
        {
            SetConsoleTextAttribute(console, attr);
        }

        printf("%s", buffer);

        if (NULL != console && IS_FALSE == console_is_plain_mode)
        {
            SetConsoleTextAttribute(console, default_attr);
        }
    }
}

void os_print_prompt(void)
{
    os_print(LIGHT_WHITE, "\r: ");
}

bool_t os_remove_timer(os_timer_id id)
{
    return SDL_RemoveTimer(id);
}

uint64 os_swap_64(uint64 n)
{
    return SDL_Swap64(n);
}

uint32 os_swap_be_32(uint32 n)
{
    return SDL_SwapBE32(n);
}

void os_quit(void)
{
    SDL_Quit();
}
