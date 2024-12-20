/** @file test_os.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cmocka.h"
#include "os.h"
#include "test_os.h"

static int sum_values(int count, ...);

void test_os_atof(void** state)
{
    const char* str = "123.45";
    double      result;

    (void)state;

    result = os_atof(str);

    assert_true(result == 123.45);
}

void test_os_atoi(void** state)
{
    int         result;
    const char* str = "12345";

    (void)state;

    result = os_atoi(str);

    assert_int_equal(result, 12345);
}

void test_os_calloc(void** state)
{
    size_t nmemb = 5;
    size_t size  = sizeof(int);
    void*  ptr   = os_calloc(nmemb, size);

    (void)state;

    assert_non_null(ptr);

    os_free(ptr);
}

void test_os_clock(void** state)
{
    clock_t result;

    (void)state;

    result = os_clock();

    assert_true(result >= 0);
}

void test_os_closedir(void** state)
{
    int    result;
    DIR_t* dir = os_opendir(".");

    (void)state;

    assert_non_null(dir);

    result = os_closedir(dir);

    assert_int_equal(result, 0);
}

void test_os_delay(void** state)
{
    uint64 time = os_get_ticks();
    uint64 new_time;

    os_delay(100);
    time     = time + 100u;
    new_time = os_get_ticks();

    /* Max. deviation is 1ms. */
    assert_in_range(new_time, time, time + 1u);
}

void test_os_fclose(void** state)
{
    int     result;
    FILE_t* file = os_fopen("fclose.txt", "w");

    (void)state;

    assert_non_null(file);

    result = os_fclose(file);

    assert_int_equal(result, 0);
}

void test_os_fgets(void** state)
{
    FILE_t* file = os_fopen("tests/test.txt", "r");
    char    buffer[20];
    char*   result;

    (void)state;

    assert_non_null(file);

    result = os_fgets(buffer, sizeof(buffer), file);

    assert_non_null(result);

    os_fclose(file);
}

void test_os_fopen(void** state)
{
    FILE_t* file = os_fopen("fopen.txt", "w");

    (void)state;

    assert_non_null(file);

    os_fclose(file);
}

void test_os_fread(void** state)
{
    FILE_t* file = os_fopen("tests/test.txt", "r");
    char    buffer[20];
    size_t  result;

    (void)state;

    assert_non_null(file);

    result = os_fread(buffer, 1, sizeof(buffer), file);

    assert_true(result > 0);

    os_fclose(file);
}

void test_os_freopen(void** state)
{
    FILE_t* file = os_fopen("freopen.txt", "w");

    (void)state;

    assert_non_null(file);

    file = os_freopen("tests/test.txt", "r", file);

    assert_non_null(file);

    os_fclose(file);
}

void test_os_fseek(void** state)
{
    int     result;
    FILE_t* file = os_fopen("tests/test.txt", "r");

    (void)state;

    assert_non_null(file);

    result = os_fseek(file, 0, SEEK_SET);

    assert_int_equal(result, 0);

    os_fclose(file);
}

void test_os_ftell(void** state)
{
    long    result;
    FILE_t* file = os_fopen("tests/test.txt", "r");

    (void)state;

    assert_non_null(file);

    os_fseek(file, 0, SEEK_END);
    result = os_ftell(file);

    assert_true(result >= 0);

    os_fclose(file);
}

void test_os_free(void** state)
{
    void* ptr = malloc(sizeof(int));

    (void)state;

    assert_non_null(ptr);

    os_free(ptr);
}

void test_os_get_error(void** state)
{
    assert_non_null(os_get_error());
}

void test_os_get_ticks(void** state)
{
    uint64 time_a = os_get_ticks();
    uint64 time_b;
    os_delay(1);
    time_b = os_get_ticks();

    assert_true(time_b > time_a);
}

void test_os_isdigit(void** state)
{
    (void)state;

    assert_int_not_equal(os_isdigit('4'), 0);
    assert_int_not_equal(os_isdigit('2'), 0);
    assert_int_equal(os_isdigit('a'), 0);
    assert_int_equal(os_isdigit('-'), 0);
}

void test_os_isprint(void** state)
{
    (void)state;

    assert_int_not_equal(os_isprint('4'), 0);
    assert_int_not_equal(os_isprint('2'), 0);
    assert_int_equal(os_isprint('\r'), 0);
    assert_int_equal(os_isprint('\n'), 0);
}

void test_os_isspace(void** state)
{
    (void)state;

    assert_int_not_equal(os_isspace(' '), 0);
    assert_int_equal(os_isspace('a'), 0);
}

void test_os_isxdigit(void** state)
{
    (void)state;

    assert_int_not_equal(os_isxdigit('A'), 0);
    assert_int_equal(os_isxdigit('G'), 0);
}

void test_os_itoa(void** state)
{
    int  value = 12345;
    char buffer[20];

    (void)state;

    os_itoa(value, buffer, 10);

    assert_string_equal(buffer, "12345");
}

void test_os_memcpy(void** state)
{
    const char src[] = "Hello, world!";
    char       dest[sizeof(src)];

    (void)state;

    os_memcpy(dest, src, sizeof(src));

    assert_memory_equal(dest, src, sizeof(src));
}

void test_os_memmove(void** state)
{
    char str1[] = "Forty";
    char str2[] = "Two";

    (void)state;

    assert_string_equal(str1, "Forty");
    assert_string_equal(str2, "Two");

    os_memmove(str1, str2, sizeof(str2));

    assert_string_equal(str1, "Two");
}

void test_os_memset(void** state)
{
    char buffer[20];

    (void)state;

    os_memset(buffer, 'A', sizeof(buffer));

    assert_memory_equal(buffer, "AAAAAAAAAAAAAAAAAAAA", sizeof(buffer));
}

void test_os_opendir(void** state)
{
    DIR_t* dir = os_opendir(".");

    (void)state;

    assert_non_null(dir);

    os_closedir(dir);
}

void test_os_readdir(void** state)
{
    DIR_t* dir = os_opendir(".");
    void*  entry;

    (void)state;

    assert_non_null(dir);

    entry = os_readdir(dir);

    assert_non_null(entry);

    os_closedir(dir);
}

void test_os_realloc(void** state)
{
    int* ptr;
    int* new_ptr;

    (void)state;

    ptr = (int*)os_calloc(5, sizeof(int));
    assert_non_null(ptr);

    new_ptr = (int*)os_realloc(ptr, 10 * sizeof(int));
    assert_non_null(new_ptr);

    os_free(new_ptr);
}

void test_os_rewind(void** state)
{
    long    result;
    FILE_t* file = os_fopen("tests/test.txt", "r");

    (void)state;

    assert_non_null(file);

    os_rewind(file);

    result = os_ftell(file);

    assert_int_equal(result, 0);

    os_fclose(file);
}

void test_os_snprintf(void** state)
{
    char buffer[20];
    int  result = os_snprintf(buffer, sizeof(buffer), "%d %s", 123, "hello");

    (void)state;

    assert_int_not_equal(result, -1);
}

void test_os_strchr(void** state)
{
    const char* str    = "Hello, world!";
    char*       result = os_strchr(str, 'w');

    (void)state;

    assert_non_null(result);
    assert_string_equal(result, "world!");
}

void test_os_strcmp(void** state)
{
    const char* str1 = "Hello";
    const char* str2 = "Hello";
    const char* str3 = "World";

    (void)state;

    assert_int_equal(os_strcmp(str1, str2), 0);
    assert_true(os_strcmp(str1, str3) < 0);
    assert_true(os_strcmp(str3, str1) > 0);
}

void test_os_strcspn(void** state)
{
    (void)state;

    {
        const char* s      = "Hello World";
        const char* reject = "aeiou";
        size_t      result = os_strcspn(s, reject);

        assert_int_equal(result, 1);
    }

    {
        const char* s      = "Hello World";
        const char* reject = "aeiou";
        size_t      result = os_strcspn(s, reject);

        assert_int_equal(result, 1);
    }

    {
        const char* s      = "Hello World";
        const char* reject = "";
        size_t      result = os_strcspn(s, reject);

        assert_int_equal(result, os_strlen(s));
    }

    {
        const char* s      = "Hello";
        const char* reject = "xyz";
        size_t      result = os_strcspn(s, reject);

        assert_int_equal(result, os_strlen(s));
    }
}

void test_os_strdup(void** state)
{
    const char* str  = "Hello, world!";
    char*       copy = os_strdup(str);

    (void)state;

    assert_non_null(copy);
    assert_string_equal(copy, str);

    os_free(copy);
}

void test_os_strlcat(void** state)
{
    char        dest[20] = "Hello";
    const char* src      = ", world!";
    size_t      result   = os_strlcat(dest, src, sizeof(dest));

    (void)state;

    assert_int_equal(result, strlen("Hello, world!"));
    assert_string_equal(dest, "Hello, world!");
}

void test_os_strlcpy(void** state)
{
    char        dest[20];
    const char* src = "Hello, world!";
    size_t      result;

    (void)state;

    result = os_strlcpy(dest, src, sizeof(dest));

    assert_int_equal(result, strlen(src));
    assert_string_equal(dest, src);
}

void test_os_strlen(void** state)
{
    const char* str = "Hello, world!";
    size_t      len = os_strlen(str);

    (void)state;

    assert_int_equal(len, strlen(str));
}

void test_os_strncmp(void** state)
{
    const char* str1 = "Hello";
    const char* str2 = "Hello";
    const char* str3 = "World";

    (void)state;

    assert_int_equal(os_strncmp(str1, str2, 5), 0);
    assert_true(os_strncmp(str1, str3, 5) < 0);
    assert_true(os_strncmp(str3, str1, 5) > 0);
}

void test_os_strrchr(void** state)
{
    const char* str    = "Hello, world!";
    char*       result = os_strrchr(str, 'o');

    (void)state;

    assert_non_null(result);
    assert_string_equal(result, "orld!");
}

void test_os_strstr(void** state)
{
    const char* haystack = "Hello, world!";
    const char* needle   = "world";
    char*       result;

    (void)state;

    result = os_strstr(haystack, needle);

    assert_non_null(result);
    assert_string_equal(result, "world!");
}

void test_os_strtokr(void** state)
{
    char  str[] = "A,B,C,D";
    char* saveptr;
    char* token = os_strtokr(str, ",", &saveptr);

    (void)state;

    assert_non_null(token);
    assert_string_equal(token, "A");

    token = os_strtokr(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "B");

    token = os_strtokr(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "C");

    token = os_strtokr(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "D");

    token = os_strtokr(NULL, ",", &saveptr);
    assert_null(token);
}

void test_os_strtol(void** state)
{
    const char* str = "12345";
    char*       endptr;
    long        result = os_strtol(str, &endptr, 10);

    (void)state;

    assert_int_equal(result, 12345);
    assert_ptr_equal(endptr, str + strlen(str));
}

void test_os_strtoul(void** state)
{
    const char*   str = "12345";
    char*         endptr;
    unsigned long result;

    (void)state;

    result = os_strtoul(str, &endptr, 10);

    assert_int_equal(result, 12345);
    assert_ptr_equal(endptr, str + strlen(str));
}

void test_os_strtoull(void** state)
{
    const char*        str = "1234567890";
    char*              endptr;
    unsigned long long result = os_strtoull(str, &endptr, 10);

    (void)state;

    assert_int_equal(result, 1234567890);
    assert_ptr_equal(endptr, str + strlen(str));
}

void test_os_swap_be_32(void** state)
{
    (void)state;
    assert_int_equal(os_swap_be_32(0x78563412), 0x12345678);
    assert_int_equal(os_swap_be_32(0x12345678), 0x78563412);
}

void test_os_tolower(void** state)
{
    (void)state;

    assert_int_equal(os_tolower('A'), 'a');
    assert_int_equal(os_tolower('a'), 'a');
}

void test_os_toupper(void** state)
{
    (void)state;

    assert_int_equal(os_toupper('a'), 'A');
    assert_int_equal(os_toupper('A'), 'A');
}

void test_uint16(void** state)
{
    (void)state;
    assert_int_equal(sizeof(uint16), 2);
}

void test_uint32(void** state)
{
    (void)state;
    assert_int_equal(sizeof(uint32), 4);
}

void test_uint64(void** state)
{
    (void)state;
    assert_int_equal(sizeof(uint64), 8);
}

void test_uint8(void** state)
{
    (void)state;
    assert_int_equal(sizeof(uint8), 1);
}

void test_variadic_functions(void** state)
{
    (void)state;

    {
        int result = sum_values(2, 3, 5);
        assert_int_equal(result, 8);
    }

    {
        int result = sum_values(3, 1, 2, 3);
        assert_int_equal(result, 6);
    }

    {
        int result = sum_values(5, 1, 2, 3, 4, 5);
        assert_int_equal(result, 15);
    }
}

static int os_vsnprintf_wrapper(char* str, size_t size, const char* format, ...)
{
    int       result;
    va_list_t args;

    os_va_start(args, format);
    result = os_vsnprintf(str, size, format, args);
    os_va_end(args);

    return result;
}

void test_os_vsnprintf(void** state)
{
    int         result;
    char        buffer[100];
    const char* format  = "%s %d. %s";
    const char* value_1 = "6 by 9.";
    int         value_2 = 42;
    const char* value_3 = "That's it. That's all there is";

    (void)state;

    result = os_vsnprintf_wrapper(buffer, sizeof(buffer), format, value_1, value_2, value_3);

    assert_string_equal(buffer, "6 by 9. 42. That's it. That's all there is");
    assert_int_equal(result, 42);
}

static int sum_values(int count, ...)
{
    int       i;
    va_list_t args;
    int       sum = 0;

    os_va_start(args, count);

    for (i = 0; i < count; ++i)
    {
        sum += os_va_arg(args, int);
    }

    os_va_end(args);

    return sum;
}
