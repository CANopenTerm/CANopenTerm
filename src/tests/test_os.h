/** @file test_os.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_OS_H
#define TEST_OS_H

void test_os_atof(void** state);
void test_os_atoi(void** state);
void test_os_calloc(void** state);
void test_os_clock(void** state);
void test_os_closedir(void** state);
void test_os_delay(void** state);
void test_os_fclose(void** state);
void test_os_fgets(void** state);
void test_os_fopen(void** state);
void test_os_fread(void** state);
void test_os_free(void** state);
void test_os_freopen(void** state);
void test_os_fseek(void** state);
void test_os_ftell(void** state);
void test_os_get_error(void** state);
void test_os_get_ticks(void** state);
void test_os_isdigit(void** state);
void test_os_isprint(void** state);
void test_os_isspace(void** state);
void test_os_isxdigit(void** state);
void test_os_itoa(void** state);
void test_os_memcpy(void** state);
void test_os_memmove(void** state);
void test_os_memset(void** state);
void test_os_opendir(void** state);
void test_os_readdir(void** state);
void test_os_realloc(void** state);
void test_os_rewind(void** state);
void test_os_snprintf(void** state);
void test_os_strchr(void** state);
void test_os_strcmp(void** state);
void test_os_strcspn(void** state);
void test_os_strdup(void** state);
void test_os_strlcat(void** state);
void test_os_strlcpy(void** state);
void test_os_strlen(void** state);
void test_os_strncmp(void** state);
void test_os_strrchr(void** state);
void test_os_strstr(void** state);
void test_os_strtokr_r(void** state);
void test_os_strtol(void** state);
void test_os_strtoul(void** state);
void test_os_strtoull(void** state);
void test_os_swap_be_32(void** state);
void test_os_tolower(void** state);
void test_os_toupper(void** state);
void test_os_vsnprintf(void** state);
void test_uint16(void** state);
void test_uint32(void** state);
void test_uint64(void** state);
void test_uint8(void** state);
void test_variadic_functions(void** state);

#endif /* TEST_OS_H */
