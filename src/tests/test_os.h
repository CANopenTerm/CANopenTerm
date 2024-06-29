/** @file test_os.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_OS_H
#define TEST_OS_H

void test_os_calloc(void** state);
void test_os_free(void** state);
void test_os_isdigit(void** state);
void test_os_isprint(void** state);
void test_os_itoa(void** state);
void test_os_memcpy(void** state);
void test_os_memmove(void** state);
void test_os_memset(void** state);
void test_os_realloc(void** state);
void test_os_snprintf(void** state);
void test_os_strchr(void** state);
void test_os_strcmp(void** state);
void test_os_strdup(void** state);
void test_os_strlcat(void** state);
void test_os_strlen(void** state);
void test_os_strncmp(void** state);
void test_os_strrchr(void** state);
void test_os_strtokr(void** state);
void test_os_strtol(void** state);
void test_os_strtoull(void** state);
void test_os_vsnprintf(void** state);
void test_uint8(void** state);
void test_uint16(void** state);
void test_uint32(void** state);
void test_uint64(void** state);

#endif /* TEST_OS_H */
