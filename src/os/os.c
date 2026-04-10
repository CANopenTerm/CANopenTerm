/** @file os.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <isocline.h>

#include "os.h"

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
