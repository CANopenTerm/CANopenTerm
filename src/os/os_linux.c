/** @file os_linux.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <SDL3/SDL.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "buffer.h"
#include "command.h"
#include "dirent.h"
#include "icon.h"
#include "os.h"
#include "palette.h"

static bool console_is_plain_mode;

static void set_nonblocking(int fd, int nonblocking);
static void set_terminal_raw_mode(struct termios* orig_termios);
static void reset_terminal_mode(struct termios* orig_termios);

os_timer_id os_add_timer(uint64 interval, os_timer_cb callback, void* param)
{
    return SDL_AddTimerNS(interval, callback, param);
}

status_t os_console_init(bool is_plain_mode)
{
    console_is_plain_mode = is_plain_mode;
    return ALL_OK;
}

static SDL_Tray* tray = NULL;
static os_window* tray_window = NULL;
static SDL_Surface* tray_icon_surface = NULL;
static bool should_show_console = false;
static bool tray_enabled = true;

static SDL_Surface* icon_bmp_data = NULL; /* Keep reference for clean-up. */

static SDL_Surface* os_load_icon_surface(void)
{
    SDL_IOStream* io;
    SDL_Surface* surface = NULL;
    unsigned char offset_bytes[4];
    uint32_t bmp_offset;
    uint32_t bmp_size;
    unsigned char bmp_file_header[14];
    unsigned char* bmp_data;
    size_t bytes_read;
    uint32_t pixel_offset;

    /* Create a read-only I/O stream from the embedded icon data. */
    io = SDL_IOFromMem((void*)icon_ico, icon_ico_len);
    if (NULL == io)
    {
        return NULL;
    }

    /* Parse .ico header to get BMP offset and size. */
    if (SDL_SeekIO(io, 14, SDL_IO_SEEK_SET) < 0)
    {
        SDL_CloseIO(io);
        return NULL;
    }

    /* Read BMP size (dwBytesInRes at offset 14-17). */
    if (SDL_ReadIO(io, offset_bytes, 4) != 4)
    {
        SDL_CloseIO(io);
        return NULL;
    }
    bmp_size = offset_bytes[0] | (offset_bytes[1] << 8) |
               (offset_bytes[2] << 16) | (offset_bytes[3] << 24);

    /* Read BMP offset (dwImageOffset at offset 18-21) */
    if (SDL_ReadIO(io, offset_bytes, 4) != 4)
    {
        SDL_CloseIO(io);
        return NULL;
    }
    bmp_offset = offset_bytes[0] | (offset_bytes[1] << 8) |
                 (offset_bytes[2] << 16) | (offset_bytes[3] << 24);

    /* For a standard 32-bit BMP with no color palette:
     * - BMP file header: 14 bytes
     * - BITMAPINFOHEADER: 40 bytes
     * - Pixel data starts at: 14 + 40 = 54 bytes from file start
     */
    pixel_offset = 54;

    /* Allocate buffer for full BMP file (header + icon data). */
    bmp_data = (unsigned char*)SDL_malloc(14 + bmp_size);
    if (NULL == bmp_data)
    {
        SDL_CloseIO(io);
        return NULL;
    }

    /* Create BMP file header. */
    bmp_file_header[0] = 'B';
    bmp_file_header[1] = 'M';
    uint32_t file_size = 14 + bmp_size;
    bmp_file_header[2] = (unsigned char)(file_size & 0xFF);
    bmp_file_header[3] = (unsigned char)((file_size >> 8) & 0xFF);
    bmp_file_header[4] = (unsigned char)((file_size >> 16) & 0xFF);
    bmp_file_header[5] = (unsigned char)((file_size >> 24) & 0xFF);
    bmp_file_header[6] = 0;
    bmp_file_header[7] = 0;
    bmp_file_header[8] = 0;
    bmp_file_header[9] = 0;
    bmp_file_header[10] = (unsigned char)(pixel_offset & 0xFF);
    bmp_file_header[11] = (unsigned char)((pixel_offset >> 8) & 0xFF);
    bmp_file_header[12] = (unsigned char)((pixel_offset >> 16) & 0xFF);
    bmp_file_header[13] = (unsigned char)((pixel_offset >> 24) & 0xFF);

    /* Copy header to buffer. */
    for (int i = 0; i < 14; i++)
    {
        bmp_data[i] = bmp_file_header[i];
    }

    /* Read BMP data from icon. */
    if (SDL_SeekIO(io, bmp_offset, SDL_IO_SEEK_SET) < 0)
    {
        SDL_free(bmp_data);
        SDL_CloseIO(io);
        return NULL;
    }

    bytes_read = SDL_ReadIO(io, bmp_data + 14, bmp_size);
    if (bytes_read != bmp_size)
    {
        SDL_free(bmp_data);
        SDL_CloseIO(io);
        return NULL;
    }

    /* Fix the height field in BITMAPINFOHEADER
     * ICO format stores height as 2x (for AND mask), but for BMP we need the actual height.
     * Height is at offset 14+8 = 22 in the combined buffer.
     */
    int32_t height_offset = 22;
    int32_t ico_height = (int32_t)(bmp_data[height_offset] | (bmp_data[height_offset + 1] << 8) |
                                   (bmp_data[height_offset + 2] << 16) | (bmp_data[height_offset + 3] << 24));
    int32_t bmp_height = ico_height / 2;

    bmp_data[height_offset] = (unsigned char)(bmp_height & 0xFF);
    bmp_data[height_offset + 1] = (unsigned char)((bmp_height >> 8) & 0xFF);
    bmp_data[height_offset + 2] = (unsigned char)((bmp_height >> 16) & 0xFF);
    bmp_data[height_offset + 3] = (unsigned char)((bmp_height >> 24) & 0xFF);

    SDL_CloseIO(io);

    /* Create SDL_IOStream from the complete BMP data; don't free on close. */
    SDL_IOStream* bmp_io = SDL_IOFromMem((void*)bmp_data, 14 + bmp_size);
    if (NULL == bmp_io)
    {
        SDL_free(bmp_data);
        return NULL;
    }

    /* Load BMP surface; pass SDL_FALSE to NOT close the stream. */
    surface = SDL_LoadBMP_IO(bmp_io, false);
    SDL_CloseIO(bmp_io); /* Close manually */

    if (NULL == surface)
    {
        SDL_free(bmp_data);
        return NULL;
    }

    /* Keep bmp_data alive until surface is destroyed. */
    icon_bmp_data = (SDL_Surface*)bmp_data; /* Store for later clean-up. */

    return surface;
}

static void os_tray_show_callback(void* userdata, SDL_TrayEntry* entry)
{
    (void)entry;
    (void)userdata;

    /* Set flag to show console; don't call os_console_show directly
     * since we're in a tray event callback and destroying the tray
     * during event handling can cause issues.
     */
    should_show_console = true;
}

static void os_tray_hide_callback(void* userdata, SDL_TrayEntry* entry)
{
    (void)entry;
    (void)userdata;

    /* Set flag to hide console; don't call os_console_hide directly
     * since we're in a tray event callback and destroying the tray
     * during event handling can cause issues.
     */
    should_show_console = false;
}

static void os_tray_quit_callback(void* userdata, SDL_TrayEntry* entry)
{
    (void)entry;
    (void)userdata;

    SDL_Event e;
    e.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&e);
}

static void os_tray_create_internal(void)
{
    SDL_TrayMenu* menu;
    SDL_TrayEntry* entry;

    if (NULL == tray_window)
    {
        return;
    }

    if (false == tray_enabled)
    {
        return;
    }

    if (NULL != tray)
    {
        return;
    }

    /* Load the icon surface from embedded data. */
    if (NULL == tray_icon_surface)
    {
        tray_icon_surface = os_load_icon_surface();
    }

    /* Create tray with the icon surface (or NULL if icon loading fails). */
    tray = SDL_CreateTray(tray_icon_surface, "CANopenTerm");

    if (NULL != tray)
    {
        menu = SDL_CreateTrayMenu(tray);

        if (NULL != menu)
        {
            entry = SDL_InsertTrayEntryAt(menu, -1, "Show", SDL_TRAYENTRY_BUTTON);
            if (NULL != entry)
            {
                SDL_SetTrayEntryCallback(entry, os_tray_show_callback, NULL);
            }

            entry = SDL_InsertTrayEntryAt(menu, -1, "Hide", SDL_TRAYENTRY_BUTTON);
            if (NULL != entry)
            {
                SDL_SetTrayEntryCallback(entry, os_tray_hide_callback, NULL);
            }

            entry = SDL_InsertTrayEntryAt(menu, -1, "Quit", SDL_TRAYENTRY_BUTTON);
            if (NULL != entry)
            {
                SDL_SetTrayEntryCallback(entry, os_tray_quit_callback, NULL);
            }
        }
    }
}

static void os_tray_destroy_internal(void)
{
    if (NULL != tray)
    {
        SDL_DestroyTray(tray);
        tray = NULL;
    }

    if (NULL != tray_icon_surface)
    {
        SDL_DestroySurface(tray_icon_surface);
        tray_icon_surface = NULL;
    }

    if (NULL != icon_bmp_data)
    {
        SDL_free(icon_bmp_data);
        icon_bmp_data = NULL;
    }
}

void os_set_tray_window(os_window* window)
{
    tray_window = window;
}

void os_disable_tray(void)
{
    tray_enabled = false;
}

void os_console_hide()
{
    if (true == tray_enabled)
    {
        os_tray_create_internal();
    }
}

void os_console_show()
{
    /* On Linux, console is typically always shown. */
}

void os_update_tray_state(void)
{
    if (should_show_console)
    {
        should_show_console = false;
        os_console_show();
    }
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

int os_wait_thread(os_thread* thread)
{
    int result = 0;
    if (thread != NULL)
    {
        SDL_WaitThread(thread, &result);
    }
    return result;
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
    int num_drivers;
    const char* selected_driver = NULL;

    /* Initialize SDL if not already initialized */
    if (! SDL_WasInit(SDL_INIT_VIDEO))
    {
        if (! SDL_Init(0)) /* Initialize SDL without any subsystems */
        {
            return OS_INIT_ERROR;
        }
    }

    num_drivers = SDL_GetNumVideoDrivers();

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

    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, "de.canopenterm.CANopenTerm");

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

static struct termios raw_saved_termios;
static bool raw_is_active = false;

/* Unbuffered, unechoed input, e.g. for remote terminal sessions. */
bool os_console_raw_enter(void)
{
    if (true == raw_is_active)
    {
        return true;
    }

    if (0 != tcgetattr(STDIN_FILENO, &raw_saved_termios))
    {
        return false;
    }

    set_terminal_raw_mode(&raw_saved_termios);
    raw_is_active = true;

    return true;
}

void os_console_raw_leave(void)
{
    if (false == raw_is_active)
    {
        return;
    }

    reset_terminal_mode(&raw_saved_termios);
    raw_is_active = false;
}

int os_console_getch(void)
{
    unsigned char key;
    fd_set fds;
    struct timeval timeout = {0, 0};

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout) <= 0)
    {
        return -1;
    }

    if (read(STDIN_FILENO, &key, 1) <= 0)
    {
        return -1;
    }

    return key;
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
            os_print(DEFAULT_COLOR, "[INFO]    ");
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

    os_forward_output(color, buffer);

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
    os_snprintf(path, 256, "%s%s", SDL_GetUserFolder(SDL_FOLDER_HOME), ".CANopenTerm.history");
    ic_set_prompt_marker("\r[white]:[/] ", NULL);
    ic_set_history(path, -1);
    ic_enable_auto_tab(true);
    ic_set_default_completer(completion_callback, NULL);
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

float os_cos(float x)
{
    return SDL_cos(x);
}

float os_sin(float x)
{
    return SDL_sin(x);
}

os_memory_hooks os_get_allocator_hooks(void)
{
    os_memory_hooks hooks = {
        .malloc_fn = SDL_malloc,
        .free_fn = SDL_free};
    return hooks;
}

void os_destroy_renderer(os_renderer* renderer)
{
    if (renderer != NULL)
    {
        SDL_DestroyRenderer(renderer);
    }
}

void os_destroy_window(os_window* window)
{
    if (window != NULL)
    {
        SDL_DestroyWindow(window);
    }
}
