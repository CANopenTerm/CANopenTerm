/** @file os.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef OS_H
#define OS_H

#include <stdarg.h>
#ifdef _WIN32
#include "os_windows.h"
#elif defined __linux__
#include "os_linux.h"
#endif

#define PROMPT_BUFFER_SIZE 1024

typedef enum color
{
    DEFAULT_COLOR = 0,
    DARK_BLACK,
    DARK_BLUE,
    DARK_GREEN,
    DARK_CYAN,
    DARK_RED,
    DARK_MAGENTA,
    DARK_YELLOW,
    DARK_WHITE,
    LIGHT_BLACK,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_YELLOW,
    LIGHT_WHITE

} color_t;

typedef enum log_level
{
    LOG_DEFAULT = 0,
    LOG_SUPPRESS,
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARNING,
    LOG_ERROR

} log_level_t;

typedef enum status
{
    ALL_OK = 0,
    CORE_QUIT,
    NMT_UNKNOWN_COMMAND,
    OS_CONSOLE_INIT_ERROR,
    OS_INIT_ERROR,
    OS_MEMORY_ALLOCATION_ERROR,
    SCRIPT_INIT_ERROR,

} status_t;

#ifndef IS_TRUE
#define IS_TRUE 1
#endif

#ifndef IS_FALSE
#define IS_FALSE 0
#endif

#ifndef OS_LAYER_DEFINED
/* Error: No OS-layer defined. */
#endif

os_timer_id os_add_timer(uint32 interval, os_timer_cb callback, void* param);
status_t    os_console_init(void);
os_thread*  os_create_thread(os_thread_func fn, const char* name, void* data);
void        os_delay(uint32 delay_in_ms);
void        os_detach_thread(os_thread* thread);
const char* os_get_error(void);
status_t    os_get_prompt(char prompt[PROMPT_BUFFER_SIZE]);
uint64      os_get_ticks(void);
status_t    os_init(void);
void        os_log(const log_level_t level, const char* format, ...);
void        os_printf(const color_t color, const char* format, ...);
void        os_print_prompt(void);
bool_t      os_remove_timer(os_timer_id id);
uint32      os_swap_be_32(uint32 n);
void        os_quit(void);

#endif /* OS_H */
