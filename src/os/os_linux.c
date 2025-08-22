/** @file os_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL3/SDL.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "buffer.h"
#include "dirent.h"
#include "os.h"
#include "crossline.h"
#include "palette.h"

static bool console_is_plain_mode;

static void set_nonblocking(int fd, int nonblocking);
static void set_terminal_raw_mode(struct termios* orig_termios);
static void reset_terminal_mode(struct termios* orig_termios);

static void completion_callback(const char* buf, crossline_completions_t* lc)
{
    if (buf[0] == '\0')
    {
        // Empty line TAB -> suggest commands.
        crossline_completion_add(lc, "h", "Show full help");
        crossline_completion_add(lc, "b", "Set baud rate");
        crossline_completion_add(lc, "d", "Load data base");
        crossline_completion_add(lc, "d", "Lookup dictionary");
        crossline_completion_add(lc, "y", "Set CAN channel");
        crossline_completion_add(lc, "c", "Clear output");
        crossline_completion_add(lc, "l", "List scripts");
        crossline_completion_add(lc, "s", "Run script");
        crossline_completion_add(lc, "n", "NMT command");
        crossline_completion_add(lc, "r", "Read SDO");
        crossline_completion_add(lc, "w", "Write SDO");
        crossline_completion_add(lc, "p", "PDO");
        crossline_completion_add(lc, "q", "Quit");
    }
}

os_timer_id os_add_timer(uint64 interval, os_timer_cb callback, void* param)
{
    return SDL_AddTimerNS(interval, callback, param);
}

status_t os_console_init(bool is_plain_mode)
{
    console_is_plain_mode = is_plain_mode;
    return ALL_OK;
}

void os_console_hide()
{
    /* Not yet implemented. */
}

void os_console_show()
{
    /* Not yet implemented. */
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

const char* os_find_data_path(void)
{
    size_t i;

    const char* search_paths[2] =
        {
            "/usr/share/CANopenTerm",
            "/usr/local/share/CANopenTerm"};

    for (i = 0; i < sizeof(search_paths) / sizeof(search_paths[0]); i++)
    {
        DIR_t* d = os_opendir(search_paths[i]);
        if (d)
        {
            os_closedir(d);
            return search_paths[i];
        }
    }

    return ".";
}

const char* os_get_error(void)
{
    return SDL_GetError();
}

status_t os_get_prompt(char prompt[PROMPT_BUFFER_SIZE])
{
    status_t status = ALL_OK;

    if (NULL != crossline_readline(": ", prompt, PROMPT_BUFFER_SIZE))
    {
        SDL_strlcpy(prompt, prompt, PROMPT_BUFFER_SIZE);
        prompt[PROMPT_BUFFER_SIZE - 1] = '\0';
    }
    else
    {
        status = 1;
    }

    return status;
}

uint64 os_get_ticks(void)
{
    return SDL_GetTicksNS();
}

const char* os_get_user_directory(void)
{
    static char user_directory[PATH_MAX] = {0};
    if (0 == user_directory[0])
    {
        char* home = getenv("HOME");
        if (home)
        {
            os_strlcpy(user_directory, home, PATH_MAX - 1);
        }
    }

    return user_directory;
}

status_t os_init(void)
{
    status_t status = ALL_OK;
    int i;
    int num_drivers = SDL_GetNumVideoDrivers();
    const char* selected_driver = NULL;

    for (i = 0; i < num_drivers; i++)
    {
        const char* driver = SDL_GetVideoDriver(i);
        if (SDL_SetHint(SDL_HINT_VIDEO_DRIVER, driver) && SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            selected_driver = driver;
            break;
        }
    }

    if (! selected_driver)
    {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        if (! SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            os_log(LOG_ERROR, "Unable to initialise video sub-system: %s", os_get_error());
            status = OS_INIT_ERROR;
        }
    }

    return status;
}

bool os_key_is_hit(void)
{
    struct termios orig_termios;
    char buffer = 0;
    int n;

    set_terminal_raw_mode(&orig_termios);
    set_nonblocking(STDIN_FILENO, 1);

    n = read(STDIN_FILENO, &buffer, 1);

    reset_terminal_mode(&orig_termios);
    set_nonblocking(STDIN_FILENO, 0);

    if (n > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void os_key_send(uint16 key)
{
    /* Not yet implemented. */
}

void os_log(const log_level_t level, const char* format, ...)
{
    char buffer[1024];
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

    os_print(DEFAULT_COLOR, "%s\r\n", buffer);
}

void os_print(const color_t color, const char* format, ...)
{
    char buffer[1024];
    char print_buffer[1024];
    va_list_t varg;
    const char* color_code = "";

    switch (color)
    {
        case DEFAULT_COLOR:
            color_code = "\x1b[0m";
            break;
        case DARK_BLACK:
            color_code = "\x1b[30m";
            break;
        case DARK_BLUE:
            color_code = "\x1b[34m";
            break;
        case DARK_GREEN:
            color_code = "\x1b[32m";
            break;
        case DARK_CYAN:
            color_code = "\x1b[36m";
            break;
        case DARK_RED:
            color_code = "\x1b[31m";
            break;
        case DARK_MAGENTA:
            color_code = "\x1b[35m";
            break;
        case DARK_YELLOW:
            color_code = "\x1b[33m";
            break;
        case DARK_WHITE:
            color_code = "\x1b[37m";
            break;
        case LIGHT_BLACK:
            color_code = "\x1b[90m";
            break;
        case LIGHT_BLUE:
            color_code = "\x1b[94m";
            break;
        case LIGHT_GREEN:
            color_code = "\x1b[92m";
            break;
        case LIGHT_CYAN:
            color_code = "\x1b[96m";
            break;
        case LIGHT_RED:
            color_code = "\x1b[91m";
            break;
        case LIGHT_MAGENTA:
            color_code = "\x1b[95m";
            break;
        case LIGHT_YELLOW:
            color_code = "\x1b[93m";
            break;
        case LIGHT_WHITE:
            color_code = "\x1b[97m";
            break;
        default:
            color_code = "\x1b[0m";
            break;
    }

    os_va_start(varg, format);
    os_vsnprintf(buffer, sizeof(buffer), format, varg);
    os_va_end(varg);

    if (false == console_is_plain_mode)
    {
        os_snprintf(print_buffer, sizeof(print_buffer), "%s%s\x1b[0m", color_code, buffer);
    }
    else
    {
        os_snprintf(print_buffer, sizeof(print_buffer), "%s", buffer);
    }

    if (true == use_buffer())
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

bool os_remove_timer(os_timer_id id)
{
    return SDL_RemoveTimer(id);
}

uint64 os_swap_64(uint64 n)
{
    return SDL_Swap64(n);
}

uint32 os_swap_be_32(uint32 n)
{
    return SDL_Swap32BE(n);
}

void os_init_history(void)
{
    char path[256] = {0};
    SDL_snprintf(path, 256, "%s%s", SDL_GetUserFolder(SDL_FOLDER_HOME), "CANopenTerm.history");
    crossline_history_load(path);
    crossline_completion_register(completion_callback);
}

void os_save_history(void)
{
    char path[256] = {0};
    SDL_snprintf(path, 256, "%s%s", SDL_GetUserFolder(SDL_FOLDER_HOME), "CANopenTerm.history");
    crossline_history_save(path);
}

void os_quit(void)
{
#ifdef ENABLE_VALGRIND_SUPPORT
    /* This is useful as a debug tool to validate memory leaks,
     * but shouldn't ever be set in production applications, as
     * other libraries used by the application might use dbus
     * under the hood and this cause cause crashes if they
     * continue after SDL_Quit().
     *
     * This variable can be set to the following values :
     *
     * "0" : SDL will not call dbus_shutdown() on quit(default)
     * "1" : SDL will call dbus_shutdown() on quit
     */
    SDL_SetHintWithPriority(SDL_HINT_SHUTDOWN_DBUS_ON_QUIT, "1", SDL_HINT_OVERRIDE);
#endif
    SDL_Quit();
}

void os_clear_window(os_renderer* renderer)
{
    if (renderer)
    {
        uint8 r = (BG_COLOR & 0xff0000) >> 16;
        uint8 g = (BG_COLOR & 0x00ff00) >> 8;
        uint8 b = (BG_COLOR & 0x0000ff);

        SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
        SDL_RenderClear(renderer);
    }
}

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
