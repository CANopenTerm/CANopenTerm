/** @file os_windows.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef OS_WINDOWS_H
#define OS_WINDOWS_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "SDL_main.h"

#define os_calloc    SDL_calloc
#define os_fclose    fclose
#define os_fgets     fgets
#define os_fopen     fopen
#define os_free      SDL_free
#define os_isdigit   SDL_isdigit
#define os_isprint   SDL_isprint
#define os_isspace   SDL_isspace
#define os_itoa      SDL_itoa
#define os_memcpy    SDL_memcpy
#define os_memmove   SDL_memmove
#define os_memset    SDL_memset
#define os_printf    printf
#define os_realloc   SDL_realloc
#define os_snprintf  SDL_snprintf
#define os_strchr    SDL_strchr
#define os_strcmp    SDL_strcmp
#define os_strcspn   strcspn
#define os_strdup    SDL_strdup
#define os_strlcat   SDL_strlcat
#define os_strlen    SDL_strlen
#define os_strncmp   SDL_strncmp
#define os_strrchr   SDL_strrchr
#define os_strtokr   SDL_strtokr
#define os_strtol    SDL_strtol
#define os_strtoull  SDL_strtoull
#define os_va_arg    va_arg
#define os_va_end    va_end
#define os_va_start  va_start
#define os_vsnprintf SDL_vsnprintf

#define os_thread      SDL_Thread
#define os_thread_func SDL_ThreadFunction
#define os_timer_cb    SDL_TimerCallback
#define os_timer_id    SDL_TimerID

#define bool_t    SDL_bool
#define uint8     Uint8
#define uint16    Uint16
#define uint32    Uint32
#define uint64    Uint64
#define va_list_t va_list

#define IS_TRUE  SDL_TRUE
#define IS_FALSE SDL_FALSE

#define CLEAR_CMD "cls"

#define DEFAULT_CAN_INTERFACE ""

#define OS_LAYER_DEFINED

#endif /* OS_WINDOWS_H */
