/** @file main.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef USE_LIBSOCKETCAN
#include <string.h>
#endif
#include <stdlib.h>
#include "core.h"
#include "SDL.h"
#include "SDL_main.h"

static size_t safe_strnlen(const char* s, size_t maxlen);
static size_t strlcpy(char* dst, const char* src, size_t dstsize);

int main(int argc, char* argv[])
{
    int     status = EXIT_SUCCESS;
    core_t* core = NULL;

    if (COT_OK != core_init(&core))
    {
        status = EXIT_FAILURE;
    }

#ifdef USE_LIBSOCKETCAN
    if ((argc > 1) && (argv[1] != NULL))
    {
        strlcpy(core->can_interface, argv[1], sizeof(core->can_interface));
    }
    else
    {
        strlcpy(core->can_interface, "can0", sizeof(core->can_interface));
    }
#endif

    while (SDL_TRUE == core->is_running)
    {
        switch (core_update(core))
        {
        case COT_QUIT:
            core->is_running = SDL_FALSE;
            break;
        case COT_ERROR:
            status = EXIT_FAILURE;
            break;
        default:
            continue;
        }
    }

    core_deinit(core);
    return status;
}

static size_t safe_strnlen(const char* s, size_t maxlen)
{
    size_t len;

    for (len = 0; len < maxlen; len += 1)
    {
        if ('\0' == s[len])
        {
            break;
        }
    }

    return len;
}

static size_t strlcpy(char* dst, const char* src, size_t dstsize)
{
    size_t srclen = safe_strnlen(src, 5);
    size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;

    if (dstsize > 0)
    {
        memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }

    return srclen; // Return the length of the source string
}
