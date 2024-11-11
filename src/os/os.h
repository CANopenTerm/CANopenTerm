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
    CAN_NO_HARDWARE_FOUND,
    CAN_READ_ERROR,
    CAN_WRITE_ERROR,
    CORE_QUIT,
    EDS_OBJECT_NOT_AVAILABLE,
    EDS_PARSE_ERROR,
    ITEM_NOT_FOUND,
    NMT_UNKNOWN_COMMAND,
    NOTHING_TO_DO,
    OS_CONSOLE_INIT_ERROR,
    OS_FILE_NOT_FOUND,
    OS_INIT_ERROR,
    OS_INVALID_ARGUMENT,
    OS_MEMORY_ALLOCATION_ERROR,
    SCRIPT_INIT_ERROR

} status_t;

#ifndef os_strlcpy
size_t os_strlcpy(char* dst, const char* src, size_t dstsize);
#endif

#ifndef os_atof
#error os_atof() not defined
#endif

#ifndef os_atoi
#error os_atoi() not defined
#endif

#ifndef os_calloc
#error  os_calloc() not defined
#endif

#ifndef os_closedir
#error  os_closedir() not defined
#endif

#ifndef os_fclose
#error  os_fclose() not defined
#endif

#ifndef os_fgets
#error  os_fgets() not defined
#endif

#ifndef os_fopen
#error  os_fopen() not defined
#endif

#ifndef os_free
#error  os_free() not defined
#endif

#ifndef os_freopen
#error  os_freopen() not defined
#endif

#ifndef os_isdigit
#error  os_isdigit() not defined
#endif

#ifndef os_isprint
#error  os_isprint() not defined
#endif

#ifndef os_isspace
#error  os_isspace() not defined
#endif

#ifndef os_isxdigit
#define os_isxdigit isxdigit
#endif

#ifndef os_itoa
#error  os_itoa() not defined
#endif

#ifndef os_memcpy
#error  os_memcpy() not defined
#endif

#ifndef os_memmove
#error  os_memmove() not defined
#endif

#ifndef os_memset
#error  os_memset() not defined
#endif

#ifndef os_opendir
#error  os_opendir() not defined
#endif

#ifndef os_printf
#error  os_printf() not defined
#endif

#ifndef os_readdir
#error  os_readdir() not defined
#endif

#ifndef os_realloc
#error  os_realloc() not defined
#endif

#ifndef os_snprintf
#error  os_snprintf() not defined
#endif

#ifndef os_strchr
#error  os_strchr() not defined
#endif

#ifndef os_strcmp
#error  os_strcmp() not defined
#endif

#ifndef os_strcspn
#error  os_strcspn() not defined
#endif

#ifndef os_strdup
#error  os_strdump() not defined
#endif

#ifndef os_strlcat
#error  os_strlcat() not defined
#endif

#ifndef os_strlen
#error  os_strlen() not defined
#endif

#ifndef os_strncmp
#error  os_strncmp() not defined
#endif

#ifndef os_strlcpy
#error  os_strlcpy() not defined
#endif

#ifndef os_strrchr
#error  os_strrchr() not defined
#endif

#ifndef os_strstr
#error  os_strstr() not defined
#endif

#ifndef os_strtokr
#error  os_strtokr() not defined
#endif

#ifndef os_strtol
#error  os_strtol() not defined
#endif

#ifndef os_strtoul
#error  os_strtoul() not defined
#endif

#ifndef os_strtoull
#error  os_strtoull() not defined
#endif

#ifndef os_tolower
#error  os_tolower() not defined
#endif

#ifndef os_va_arg
#error  os_va_arg() not defined
#endif

#ifndef os_va_end
#error  os_va_end() not defined
#endif

#ifndef os_va_start
#error  os_va_start() not defined
#endif

#ifndef os_vsnprintf
#error  os_vsnprintf() not defined
#endif

#ifndef os_thread
#error  os_thread not defined
#endif

#ifndef os_thread_func
#error  os_thread_func not defined
#endif

#ifndef os_timer_cb
#error  os_timer_cb not defined
#endif

#ifndef os_timer_id
#error  os_timer_id not defined
#endif

#ifndef bool_t
#define bool_t int
#endif

#ifndef DIR_t
#error  DIR_t not defined
#endif

#ifndef dirent_t
#error  dirent_t not defined
#endif

#ifndef FILE_t
#error  FILE_t not defined
#endif

#ifndef uint8
#define uint8 char
#endif

#ifndef uint16
#error  uint16 not defined
#endif

#ifndef uint32
#error  uint32 not defined
#endif

#ifndef uint64
#error  uint64 not defined
#endif

#ifndef va_list_t
#error  va_list_t not defined
#endif

#ifndef IS_TRUE
#define IS_TRUE 1
#endif

#ifndef IS_FALSE
#define IS_FALSE 0
#endif

#ifndef CLEAR_CMD
#define CLEAR_CMD ""
#endif

#ifndef OS_LAYER_DEFINED
#error No OS - layer defined.
#endif

os_timer_id os_add_timer(uint32 interval, os_timer_cb callback, void* param);
status_t    os_console_init(bool_t is_silent);
os_thread*  os_create_thread(os_thread_func fn, const char* name, void* data);
void        os_delay(uint32 delay_in_ms);
void        os_detach_thread(os_thread* thread);
const char* os_get_error(void);
status_t    os_get_prompt(char prompt[PROMPT_BUFFER_SIZE]);
uint64      os_get_ticks(void);
status_t    os_init(void);
bool_t      os_key_is_hit(void);
void        os_log(const log_level_t level, const char* format, ...);
void        os_print(const color_t color, const char* format, ...);
void        os_print_prompt(bool_t is_silent);
bool_t      os_remove_timer(os_timer_id id);
uint64      os_swap_64(uint64 n);
uint32      os_swap_be_32(uint32 n);
void        os_quit(void);

#endif /* OS_H */
