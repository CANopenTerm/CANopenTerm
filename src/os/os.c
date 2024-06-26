/** @file os.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "os.h"

#ifndef os_strlcpy
static size_t safe_strnlen(const char* s, size_t maxlen);

size_t os_strlcpy(char* dst, const char* src, size_t dstsize)
{
    size_t srclen = safe_strnlen(src, 5);
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
