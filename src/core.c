/** @file core.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "core.h"
#include "can.h"
#include "nmt_client.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

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

static void set_gui_style(struct nk_context *ctx);

status_t core_init(core_t **core)
{
    SDL_RendererFlags flags  = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

    *core = (core_t*)calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        return COT_ERROR;
    }

    // Initialise SDL.
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(0, "Unable to initialise video sub-system: %s", SDL_GetError());
        return COT_ERROR;
    }

    (*core)->window = SDL_CreateWindow(
        "CANopenTerm",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (NULL == (*core)->window)
    {
        SDL_LogError(0, "Could not create window: %s", SDL_GetError());
        return COT_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, -1, flags);
    if (NULL == (*core)->renderer)
    {
        SDL_LogError(0, "Unable to create renderer: %s", SDL_GetError());
        return COT_ERROR;
    }

    // Initialise Nuklear.
    (*core)->ctx = nk_sdl_init((*core)->window, (*core)->renderer);
    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        // ...
        nk_sdl_font_stash_end();
    }
    set_gui_style((*core)->ctx);

    // Initialise Lua.
    (*core)->L = luaL_newstate();
    luaL_openlibs((*core)->L);

    // Initialise CAN.
    init_can((*core));

    (*core)->is_running = SDL_TRUE;
    return COT_OK;
}

status_t core_update(core_t *core)
{
    SDL_Event event;

    if (NULL == core)
    {
        return COT_OK;
    }

    // Reinitialise CAN-dongle if necessary.
    if (SDL_FALSE == is_can_initialised(core))
    {
        init_can(core);
    }

    // Handle events.
    nk_input_begin(core->ctx);
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                return COT_QUIT;
        }
        nk_sdl_handle_event(&event);
    }
    nk_input_end(core->ctx);

    // Add widgets.
    nmt_client_widget(core);

    // Update window.
    SDL_SetRenderDrawColor(core->renderer, 0xf6, 0xf8, 0xfa, 0xff);
    SDL_RenderClear(core->renderer);
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(core->renderer);

    return COT_OK;
}

void core_deinit(core_t *core)
{
    deinit_can(core);
    lua_close(core->L);
    nk_sdl_shutdown();

    if (core->window)
    {
        SDL_DestroyWindow(core->window);
    }

    if (core->renderer)
    {
        SDL_DestroyRenderer(core->renderer);
    }

    if (core)
    {
        free(core);
    }

    SDL_Quit();
}

static void set_gui_style(struct nk_context *ctx)
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
