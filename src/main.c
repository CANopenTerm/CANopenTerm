/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "config.h"

#include "SDL.h"
#include "SDL_main.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "PCANBasic.h"

extern void nmt_client_widget(struct nk_context *ctx, Uint32* can_status);

static void     set_style(struct nk_context *ctx);
static SDL_bool init_can(int* can_status);
static void     deinit_can(void);
static void     set_zoom_factor(Uint8 zoom_factor);
static Uint8    get_zoom_factor(void);

int main(int argc, char* argv[])
{
    int                status          = EXIT_SUCCESS;
    Uint32             can_status;
    SDL_bool           can_initialised = SDL_FALSE;
    SDL_bool           is_running      = SDL_TRUE;
    SDL_RendererFlags  flags           = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Window        *window;
    SDL_Renderer      *renderer;
    struct nk_context *ctx;
    lua_State         *L;

    /* Initialise SDL */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(0, "Unable to initialise video sub-system: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto cleanup;
    }

    window = SDL_CreateWindow(
        "CANopenTerm",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (NULL == window)
    {
        SDL_LogError(0, "Could not create window: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto cleanup;
    }

    renderer = SDL_CreateRenderer(window, -1, flags);
    if (NULL == renderer)
    {
        SDL_LogError(0, "Unable to create renderer: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto cleanup;
    }

    /* Initialise Nuklear */
    ctx = nk_sdl_init(window, renderer);
    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        //struct nk_font *cascadia = nk_font_atlas_add_from_file(atlas, "res/CascadiaCode.ttf", 14, 0);
        // ...
        nk_sdl_font_stash_end();
        //nk_style_set_font(ctx, &cascadia->handle);
    }
    set_style(ctx);

    /* Initialise Lua */
    L = luaL_newstate();
    luaL_openlibs(L);

    /* Initialise CAN */
    can_initialised = init_can(&can_status);

    while (SDL_TRUE == is_running)
    {
        SDL_bool  show_menu   = SDL_TRUE;
        SDL_Event event;

        if (SDL_FALSE == can_initialised)
        {
            can_initialised = init_can(&can_status);
        }

        nk_input_begin(ctx);
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    goto cleanup;
            }
            nk_sdl_handle_event(&event);
        }
        nk_input_end(ctx);

        nmt_client_widget(ctx, &can_status);

        SDL_SetRenderDrawColor(renderer, 0xf6, 0xf8, 0xfa, 0xff);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

cleanup:
    deinit_can();
    lua_close(L);
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return status;
}

static void set_style(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];

    table[NK_COLOR_TEXT]                    = nk_rgba(0x00, 0x00, 0x00, 0xff);
    table[NK_COLOR_WINDOW]                  = nk_rgba(0xff, 0xff, 0xff, 0xff);
    table[NK_COLOR_HEADER]                  = nk_rgba(0xff, 0xff, 0xff, 0xff);
    table[NK_COLOR_BORDER]                  = nk_rgba(0x11, 0x12, 0x13, 0xff);
    table[NK_COLOR_BUTTON]                  = nk_rgba(0xef, 0xf0, 0xf2, 0xff);
    table[NK_COLOR_BUTTON_HOVER]            = nk_rgba(0xe1, 0xe3, 0xe5, 0xff);
    table[NK_COLOR_BUTTON_ACTIVE]           = nk_rgba(0xd3, 0xd6, 0xd8, 0xff);
    //table[NK_COLOR_TOGGLE]                  = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_TOGGLE_HOVER]            = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_TOGGLE_CURSOR]           = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SELECT]                  = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SELECT_ACTIVE]           = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SLIDER]                  = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SLIDER_CURSOR]           = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SLIDER_CURSOR_HOVER]     = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SLIDER_CURSOR_ACTIVE]    = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_PROPERTY]                = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_EDIT]                    = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_EDIT_CURSOR]             = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_COMBO]                   = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_CHART]                   = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_CHART_COLOR]             = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_CHART_COLOR_HIGHLIGHT]   = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SCROLLBAR]               = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SCROLLBAR_CURSOR]        = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]  = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(0x, 0x, 0x, 0xff);
    //table[NK_COLOR_TAB_HEADER]              = nk_rgba(0xff, 0x00, 0x00, 0xff);

    nk_style_from_table(ctx, table);
}

static SDL_bool init_can(int* can_status)
{
    char err_message[100] = { 0 };

    *can_status = CAN_Initialize(
        PCAN_USBBUS1,
        PCAN_BAUD_250K,
        PCAN_USB,
        0, 0);

    CAN_GetErrorText(*can_status, 0x09, err_message);

    if ((*can_status & PCAN_ERROR_OK) == *can_status)
    {
        SDL_LogInfo(0, "CAN successfully initialised");
        return SDL_TRUE;
    }
    else
    {
        SDL_LogVerbose(0, "CAN could not be initialised: %s", err_message);
        return SDL_FALSE;
    }
}

static void deinit_can(void)
{
    CAN_Uninitialize(PCAN_NONEBUS);
}
