/** @file os.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <isocline.h>

#include "os.h"

static os_output_cb output_cb = NULL;

void os_set_output_cb(os_output_cb callback)
{
    output_cb = callback;
}

/* Local console output is not wanted when output is delivered to remote clients. */
void os_silence_output(void)
{
#ifdef _WIN32
    const char* null_device = "NUL";
#else
    const char* null_device = "/dev/null";
#endif

    fflush(stdout);
    if (NULL == os_freopen(null_device, "w", stdout))
    {
        os_log(LOG_WARNING, "Unable to silence local output.");
    }
}

void os_forward_output(const color_t color, const char* text)
{
    if (NULL != output_cb)
    {
        output_cb(color, text);
    }
}

char* os_fix_path(char* path)
{
    for (size_t i = 0; i < os_strlen(path); ++i)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
    return path;
}

#ifndef os_strlcpy
static size_t safe_strnlen(const char* s, size_t maxlen);

size_t os_strlcpy(char* dst, const char* src, size_t dstsize)
{
    size_t srclen = safe_strnlen(src, dstsize);
    size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;

    if (dstsize > 0)
    {
        os_memcpy(dst, src, copylen);
        dst[copylen] = '\0';
    }

    return srclen;
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
#endif

status_t os_get_prompt(char prompt[PROMPT_BUFFER_SIZE])
{
    char* buf;
    status_t status = ALL_OK;

    buf = ic_readline(NULL);
    if (buf)
    {
        os_strlcpy(prompt, buf, PROMPT_BUFFER_SIZE);
        os_free(buf);
        prompt[PROMPT_BUFFER_SIZE - 1] = '\0';
        ic_history_add((const char*)prompt);
    }
    else
    {
        status = 1;
    }

    return status;
}

void os_print_prompt(void)
{
    os_print(DEFAULT_COLOR, "\r: ");
}
