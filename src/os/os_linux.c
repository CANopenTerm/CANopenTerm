/** @file os_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "os.h"
#include <readline/readline.h>
#include <readline/history.h>

os_timer_id os_add_timer(uint32 interval, os_timer_cb callback, void* param)
{
    return SDL_AddTimer(interval, callback, param);
}

status_t os_console_init(void)
{
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
    char*    buffer;

    buffer = readline(": ");
    if (buffer != NULL)
    {
        add_history(buffer);
        os_memcpy(prompt, buffer, PROMPT_BUFFER_SIZE - 1);
        prompt[PROMPT_BUFFER_SIZE - 1] = '\0';
    }
    else
    {
        status = 1; /* TODO: Add proper error. */
    }
   
    free(buffer);
    return status;
}

uint64 os_get_ticks(void)
{
    return SDL_GetTicks64();
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
    char        buffer[1024];
    char        print_buffer[1024];
    va_list_t   varg;
    const char* color_code = "";

    switch (color)
    {
        case DEFAULT_COLOR:   color_code = "\x1b[0m"; break;
        case DARK_BLACK:      color_code = "\x1b[30m"; break;
        case DARK_BLUE:       color_code = "\x1b[34m"; break;
        case DARK_GREEN:      color_code = "\x1b[32m"; break;
        case DARK_CYAN:       color_code = "\x1b[36m"; break;
        case DARK_RED:        color_code = "\x1b[31m"; break;
        case DARK_MAGENTA:    color_code = "\x1b[35m"; break;
        case DARK_YELLOW:     color_code = "\x1b[33m"; break;
        case DARK_WHITE:      color_code = "\x1b[37m"; break;
        case LIGHT_BLACK:     color_code = "\x1b[90m"; break;
        case LIGHT_BLUE:      color_code = "\x1b[94m"; break;
        case LIGHT_GREEN:     color_code = "\x1b[92m"; break;
        case LIGHT_CYAN:      color_code = "\x1b[96m"; break;
        case LIGHT_RED:       color_code = "\x1b[91m"; break;
        case LIGHT_MAGENTA:   color_code = "\x1b[95m"; break;
        case LIGHT_YELLOW:    color_code = "\x1b[93m"; break;
        case LIGHT_WHITE:     color_code = "\x1b[97m"; break;
        default:              color_code = "\x1b[0m"; break;
    }

    os_va_start(varg, format);
    os_vsnprintf(buffer, sizeof(buffer), format, varg);
    os_va_end(varg);

    os_snprintf(print_buffer, sizeof(print_buffer), "%s%s\x1b[0m", color_code, buffer);

    if (IS_TRUE == use_buffer())
    {
        buffer_write(print_buffer);
    }
    else
    {
        os_printf("%s", print_buffer);
    }
}

void os_print_prompt(void)
{
    return;
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
