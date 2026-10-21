/** @file os_windows.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <conio.h>
#include <isocline.h>
#include <shlobj.h>
#include <windows.h>

#include "buffer.h"
#include "command.h"
#include "icon.h"
#include "os.h"
#include "palette.h"

static bool console_is_plain_mode;
static HANDLE console = NULL;
static WORD default_attr;

os_timer_id os_add_timer(uint64 interval, os_timer_cb callback, void* param)
{
    return SDL_AddTimerNS(interval, callback, param);
}

status_t os_console_init(bool is_plain_mode)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (NULL != console)
    {
        return ALL_OK;
    }

    console = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((INVALID_HANDLE_VALUE == console) || (NULL == console))
    {
        /* In MSIX environments, console may not be available.
         * This is not a fatal error; the application can still run. */
        console = NULL;
        console_is_plain_mode = is_plain_mode;
        default_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        return ALL_OK;
    }

    if (0 == GetConsoleScreenBufferInfo(console, &info))
    {
        /* Console handle is valid but cannot get info.
         * Use default attributes and continue. */
        default_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
    else
    {
        default_attr = info.wAttributes;
    }

    console_is_plain_mode = is_plain_mode;

    SetConsoleOutputCP(65001);
    SetConsoleTitle("CANopenTerm");

    return ALL_OK;
}

static SDL_Tray* tray = NULL;
static os_window* tray_window = NULL;
static SDL_Surface* tray_icon_surface = NULL;
static bool should_update_tray = false;
static bool console_is_visible = true;
static bool tray_enabled = true;

/* NOTE: Tray is created when console is hidden and displays "Show" and "Quit" buttons.
 * Tray persists until the program quits.
 */

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

    /* Allocate buffer for full BMP file (header + icon data) */
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

    /* Read BMP data from icon */
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

    /* Show console directly; safe to do since we're just calling Win32 API. */
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
    {
        ShowWindow(hwnd, SW_SHOW);
    }
    console_is_visible = true;
}

static void os_tray_hide_callback(void* userdata, SDL_TrayEntry* entry)
{
    (void)entry;
    (void)userdata;

    /* Hide console directly; safe to do since we're just calling Win32 API. */
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
    {
        ShowWindow(hwnd, SW_HIDE);
    }
    console_is_visible = false;
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

static void os_tray_menu_only_destroy(void)
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
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
    {
        ShowWindow(hwnd, SW_HIDE);
    }

    console_is_visible = false;
    if (true == tray_enabled)
    {
        os_tray_create_internal();
    }
}

void os_console_show()
{
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
    {
        ShowWindow(hwnd, SW_SHOW);
    }

    console_is_visible = true;
}

void os_update_tray_state(void)
{
    if (should_update_tray)
    {
        should_update_tray = false;
        if (true == tray_enabled && false == console_is_visible)
        {
            /* Console is hidden, make sure tray exists with "Show" button. */
            if (NULL == tray)
            {
                os_tray_create_internal();
            }
        }
    }
}

static DWORD raw_saved_in_mode;
static DWORD raw_saved_out_mode;
static bool raw_is_active = false;

/* Unbuffered, unechoed input and ANSI escape sequence output, e.g. for remote terminal sessions. */
bool os_console_raw_enter(void)
{
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (true == raw_is_active)
    {
        return true;
    }

    if (0 == GetConsoleMode(hin, &raw_saved_in_mode) || 0 == GetConsoleMode(hout, &raw_saved_out_mode))
    {
        return false;
    }

    SetConsoleMode(hin, raw_saved_in_mode & ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    SetConsoleMode(hout, raw_saved_out_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    raw_is_active = true;

    return true;
}

void os_console_raw_leave(void)
{
    if (false == raw_is_active)
    {
        return;
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), raw_saved_in_mode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), raw_saved_out_mode);
    raw_is_active = false;
}

int os_console_getch(void)
{
    int key;

    if (0 == os_kbhit())
    {
        return -1;
    }

    key = _getch();

    /* Function and cursor keys come as a prefix followed by a scan code; ignore them. */
    if (0 == key || 0xe0 == key)
    {
        (void)_getch();
        return -1;
    }

    return key;
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
    size_t len;
    static char data_path[MAX_PATH] = {0};
    const char* base_path = SDL_GetBasePath();

    if (base_path == NULL)
    {
        return ".";
    }

    os_strlcpy(data_path, base_path, MAX_PATH - 1);
    data_path[MAX_PATH - 1] = '\0';

    len = os_strlen(data_path);
    if (len > 0 && data_path[len - 1] == '\\')
    {
        data_path[len - 1] = '\0';
    }

    /* SDL_GetBasePath() returns internally cached memory that should not be freed */

    return data_path;
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
    static char user_directory[MAX_PATH] = {0};
    if (0 == user_directory[0])
    {
        const char* user_folder = SDL_GetUserFolder(SDL_FOLDER_HOME);
        if (user_folder != NULL)
        {
            os_strlcpy(user_directory, user_folder, MAX_PATH - 1);
            user_directory[MAX_PATH - 1] = '\0';
        }
        else
        {
            os_strlcpy(user_directory, ".", MAX_PATH - 1);
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
            /* In MSIX environments, SDL initialization might fail.
             * Log the error but continue; many features still work without SDL. */
            os_log(LOG_WARNING, "Unable to initialise SDL: %s", os_get_error());
            return ALL_OK;
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
            /* Video subsystem is optional - window features won't work but
             * the application can still run in console/plain mode. */
            os_log(LOG_WARNING, "Unable to initialise video sub-system: %s", os_get_error());
        }
    }

    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, "de.canopenterm.CANopenTerm");

    return status;
}

/* Internal wrapper for _kbhit() that can be mocked in tests */
int os_kbhit(void)
{
    return _kbhit();
}

bool os_key_is_hit(void)
{
    if (0 != os_kbhit())
    {
        char key = _getch();
        (void)key;

        return true;
    }
    else
    {
        return false;
    }
}

void os_key_send(uint16 key)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;

    /* Key down. */
    SendInput(1, &input, sizeof(INPUT));

    /* Key up. */
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
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
    va_list_t args;
    WORD attr = 0;

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

    os_forward_output(color, buffer);

    if (true == use_buffer())
    {
        buffer_write(buffer);
    }
    else
    {
        if (NULL != console && false == console_is_plain_mode)
        {
            SetConsoleTextAttribute(console, attr);
        }

        printf("%s", buffer);

        if (NULL != console && false == console_is_plain_mode)
        {
            SetConsoleTextAttribute(console, default_attr);
        }
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

void os_quit(void)
{
    SDL_Quit();
}

void os_init_history(void)
{
    char path[256] = {0};
    const char* user_folder = SDL_GetUserFolder(SDL_FOLDER_HOME);

    if (user_folder != NULL)
    {
        os_snprintf(path, 256, "%s/%s", user_folder, "CANopenTerm.history");
    }
    else
    {
        os_strlcpy(path, "CANopenTerm.history", 256);
    }

    ic_set_prompt_marker("\r[white]:[/] ", NULL);
    ic_set_history(path, -1);
    ic_enable_auto_tab(true);
    ic_set_default_completer(completion_callback, NULL);
}

void os_clear_window(os_renderer* renderer)
{
    if (renderer)
    {
        uint32 bg_color = palette_get_color(BG_COLOR);
        uint8 r = (bg_color & 0xff0000) >> 16;
        uint8 g = (bg_color & 0x00ff00) >> 8;
        uint8 b = (bg_color & 0x0000ff);

        SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
        SDL_RenderClear(renderer);
    }
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
