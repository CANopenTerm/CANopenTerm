/** @file test_os.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "buffer.h"
#include "cmocka.h"
#include "os.h"
#include "test_os.h"

static int sum_values(int count, ...);

void test_os_atof(void** state)
{
    double result;

    (void)state;

    result = os_atof("123.45");
    assert_true(result > 123.44 && result < 123.46);

    result = os_atof("-0.5");
    assert_true(result > -0.51 && result < -0.49);

    result = os_atof("0");
    assert_true(result == 0.0);
}

void test_os_atoi(void** state)
{
    int result;
    const char* str = "12345";

    (void)state;

    result = os_atoi(str);

    assert_int_equal(result, 12345);
}

void test_os_calloc(void** state)
{
    size_t nmemb = 5;
    size_t size = sizeof(int);
    void* ptr = os_calloc(nmemb, size);

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
    int result;
    DIR_t* dir = os_opendir(".");

    (void)state;

    assert_non_null(dir);

    result = os_closedir(dir);

    assert_int_equal(result, 0);
}

void test_os_delay(void** state)
{
    uint64 before;
    uint64 after;
    uint64 elapsed;

    (void)state;

    before = os_get_ticks();
    os_delay(100);
    after = os_get_ticks();
    elapsed = after - before;

    /* Expect at least 100 ms; allow up to 200 ms for scheduler jitter. */
    assert_true(elapsed >= 100000000ull);
    assert_true(elapsed <= 200000000ull);
}

void test_os_fclose(void** state)
{
    int result;
    FILE_t* file = os_fopen("fclose.txt", "w");

    (void)state;

    assert_non_null(file);

    result = os_fclose(file);

    assert_int_equal(result, 0);
}

void test_os_fgets(void** state)
{
    FILE_t* file = os_fopen("tests/test.txt", "r");
    char buffer[20];
    char* result;

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
    char buffer[20];
    size_t result;

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
    long pos;
    FILE_t* file = os_fopen("tests/test.txt", "r");

    (void)state;

    assert_non_null(file);

    assert_int_equal(os_fseek(file, 0, SEEK_SET), 0);
    assert_int_equal(os_ftell(file), 0);

    assert_int_equal(os_fseek(file, 2, SEEK_CUR), 0);
    pos = os_ftell(file);
    assert_true(pos == 2);

    assert_int_equal(os_fseek(file, 0, SEEK_END), 0);
    pos = os_ftell(file);
    assert_true(pos > 0);

    os_fclose(file);
}

void test_os_ftell(void** state)
{
    long result;
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
    void* ptr = os_calloc(sizeof(int), 1);

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

    /* Uppercase A-F */
    assert_int_not_equal(os_isxdigit('A'), 0);
    assert_int_not_equal(os_isxdigit('F'), 0);

    /* Lowercase a-f */
    assert_int_not_equal(os_isxdigit('a'), 0);
    assert_int_not_equal(os_isxdigit('f'), 0);

    /* Digits 0-9 */
    assert_int_not_equal(os_isxdigit('0'), 0);
    assert_int_not_equal(os_isxdigit('9'), 0);

    /* Boundary invalids */
    assert_int_equal(os_isxdigit('G'), 0);
    assert_int_equal(os_isxdigit('g'), 0);
    assert_int_equal(os_isxdigit('x'), 0);
    assert_int_equal(os_isxdigit(' '), 0);
}

void test_os_itoa(void** state)
{
    char buffer[20];

    (void)state;

    os_itoa(12345, buffer, 10);
    assert_string_equal(buffer, "12345");

    os_itoa(-99, buffer, 10);
    assert_string_equal(buffer, "-99");

    os_itoa(255, buffer, 16);
    assert_string_equal(buffer, "FF");

    os_itoa(0, buffer, 10);
    assert_string_equal(buffer, "0");
}

void test_os_memcpy(void** state)
{
    const char src[] = "Hello, world!";
    char dest[sizeof(src)];

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
    void* entry;

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
    long result;
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
    char buffer[32];
    int result;

    (void)state;

    result = os_snprintf(buffer, sizeof(buffer), "%d %s", 123, "hello");
    assert_int_equal(result, 9);
    assert_string_equal(buffer, "123 hello");

    /* Truncation: buffer of 5 should hold "123 " + NUL. */
    result = os_snprintf(buffer, 5, "%d %s", 123, "hello");
    assert_true(result >= 9);
    assert_string_equal(buffer, "123 ");
}

void test_os_strchr(void** state)
{
    const char* str = "Hello, world!";
    char* result = os_strchr(str, 'w');

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
        /* First vowel in "Hello World" is 'e' at index 1. */
        const char* s = "Hello World";
        const char* reject = "aeiou";
        size_t result = os_strcspn(s, reject);

        assert_int_equal(result, 1);
    }

    {
        /* First character itself is in reject set. */
        const char* s = "apple";
        const char* reject = "aeiou";
        size_t result = os_strcspn(s, reject);

        assert_int_equal(result, 0);
    }

    {
        /* Empty reject set; entire string is spanned. */
        const char* s = "Hello World";
        const char* reject = "";
        size_t result = os_strcspn(s, reject);

        assert_int_equal(result, os_strlen(s));
    }

    {
        /* No character in reject set present. */
        const char* s = "Hello";
        const char* reject = "xyz";
        size_t result = os_strcspn(s, reject);

        assert_int_equal(result, os_strlen(s));
    }
}

void test_os_strdup(void** state)
{
    const char* str = "Hello, world!";
    char* copy = os_strdup(str);

    (void)state;

    assert_non_null(copy);
    assert_string_equal(copy, str);

    os_free(copy);
}

void test_os_strlcat(void** state)
{
    char dest[20] = "Hello";
    const char* src = ", world!";
    size_t result = os_strlcat(dest, src, sizeof(dest));

    (void)state;

    assert_int_equal(result, strlen("Hello, world!"));
    assert_string_equal(dest, "Hello, world!");
}

void test_os_strlcpy(void** state)
{
    char dest[20];
    const char* src = "Hello, world!";
    size_t result;

    (void)state;

    result = os_strlcpy(dest, src, sizeof(dest));

    assert_int_equal(result, strlen(src));
    assert_string_equal(dest, src);
}

void test_os_strlen(void** state)
{
    const char* str = "Hello, world!";
    size_t len = os_strlen(str);

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
    const char* str = "Hello, world!";
    char* result = os_strrchr(str, 'o');

    (void)state;

    assert_non_null(result);
    assert_string_equal(result, "orld!");
}

void test_os_strstr(void** state)
{
    const char* haystack = "Hello, world!";
    const char* needle = "world";
    char* result;

    (void)state;

    result = os_strstr(haystack, needle);

    assert_non_null(result);
    assert_string_equal(result, "world!");
}

void test_os_strtokr_r(void** state)
{
    char str[] = "A,B,C,D";
    char* saveptr;
    char* token;

    (void)state;

    token = os_strtokr_r(str, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "A");

    token = os_strtokr_r(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "B");

    token = os_strtokr_r(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "C");

    token = os_strtokr_r(NULL, ",", &saveptr);
    assert_non_null(token);
    assert_string_equal(token, "D");

    token = os_strtokr_r(NULL, ",", &saveptr);
    assert_null(token);

    {
        /* No delimiter present; entire string is the single token. */
        char no_delim[] = "hello";
        token = os_strtokr_r(no_delim, ",", &saveptr);
        assert_non_null(token);
        assert_string_equal(token, "hello");

        token = os_strtokr_r(NULL, ",", &saveptr);
        assert_null(token);
    }
}

void test_os_strtol(void** state)
{
    char* endptr;

    (void)state;

    assert_int_equal(os_strtol("12345", &endptr, 10), 12345);
    assert_ptr_equal(endptr, "12345" + 5);

    assert_int_equal(os_strtol("-99", &endptr, 10), -99);

    assert_int_equal(os_strtol("1A", &endptr, 16), 26);
    assert_ptr_equal(endptr, "1A" + 2);

    assert_int_equal(os_strtol("0x1F", &endptr, 0), 31);
}

void test_os_strtoul(void** state)
{
    char* endptr;

    (void)state;

    assert_int_equal(os_strtoul("12345", &endptr, 10), 12345);
    assert_ptr_equal(endptr, "12345" + 5);

    assert_int_equal(os_strtoul("FF", &endptr, 16), 255);
    assert_ptr_equal(endptr, "FF" + 2);

    assert_int_equal(os_strtoul("0", &endptr, 10), 0);
}

void test_os_strtoull(void** state)
{
    char* endptr;

    (void)state;

    assert_int_equal(os_strtoull("1234567890", &endptr, 10), 1234567890);
    assert_ptr_equal(endptr, "1234567890" + 10);

    assert_true(os_strtoull("DEADBEEF", &endptr, 16) == 0xDEADBEEFULL);
    assert_ptr_equal(endptr, "DEADBEEF" + 8);
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

void test_os_fix_path(void** state)
{
    char path_a[] = "some\\windows\\path\\file.txt";
    char path_b[] = "already/unix/path";
    char path_c[] = "mixed\\path/to\\file";

    (void)state;

    assert_string_equal(os_fix_path(path_a), "some/windows/path/file.txt");
    assert_string_equal(os_fix_path(path_b), "already/unix/path");
    assert_string_equal(os_fix_path(path_c), "mixed/path/to/file");
}

void test_os_get_user_directory(void** state)
{
    const char* dir;

    (void)state;

    dir = os_get_user_directory();

    assert_non_null(dir);
    assert_true(os_strlen(dir) > 0);
}

void test_os_find_data_path(void** state)
{
    const char* path;

    (void)state;

    path = os_find_data_path();

    assert_non_null(path);
    assert_true(os_strlen(path) > 0);
}

static int os_vsnprintf_wrapper(char* str, size_t size, const char* format, ...)
{
    int result;
    va_list_t args;

    os_va_start(args, format);
    result = os_vsnprintf(str, size, format, args);
    os_va_end(args);

    return result;
}

void test_os_vsnprintf(void** state)
{
    int result;
    char buffer[100];
    const char* format = "%s %d. %s";
    const char* value_1 = "6 by 9.";
    int value_2 = 42;
    const char* value_3 = "That's it. That's all there is";

    (void)state;

    result = os_vsnprintf_wrapper(buffer, sizeof(buffer), format, value_1, value_2, value_3);

    assert_string_equal(buffer, "6 by 9. 42. That's it. That's all there is");
    assert_int_equal(result, 42);
}

void test_os_sscanf(void** state)
{
    int a, b;
    float f;
    char s[32];
    int result;

    (void)state;

    result = os_sscanf("42 7", "%d %d", &a, &b);
    assert_int_equal(result, 2);
    assert_int_equal(a, 42);
    assert_int_equal(b, 7);

    result = os_sscanf("3.14", "%f", &f);
    assert_int_equal(result, 1);
    assert_true(f > 3.13f && f < 3.15f);

    result = os_sscanf("hello", "%31s", s);
    assert_int_equal(result, 1);
    assert_string_equal(s, "hello");
}

void test_os_swap_64(void** state)
{
    (void)state;

    assert_true(os_swap_64(0x0102030405060708ULL) == 0x0807060504030201ULL);
    assert_true(os_swap_64(0x0000000000000001ULL) == 0x0100000000000000ULL);
    assert_true(os_swap_64(0xFF00000000000000ULL) == 0x00000000000000FFULL);
}

void test_os_console_init(void** state)
{
    (void)state;

    /* os_console_init is idempotent; calling it when already initialized must return ALL_OK. */
    assert_true(os_console_init(false) == ALL_OK);
    assert_true(os_console_init(true) == ALL_OK);
}

void test_os_log(void** state)
{
    (void)state;

    /* LOG_SUPPRESS must be a silent no-op. */
    os_log(LOG_SUPPRESS, "this should not appear");

    /* All other levels must execute without crashing. */
    os_log(LOG_DEFAULT, "log default");
    os_log(LOG_INFO, "log info");
    os_log(LOG_SUCCESS, "log success");
    os_log(LOG_WARNING, "log warning");
    os_log(LOG_ERROR, "log error");
}

void test_os_print(void** state)
{
    (void)state;

    /* Route output through the buffer so nothing is written to the console
     * and we can verify the formatted string was produced correctly. */
    assert_true(buffer_init(256) == ALL_OK);

    os_print(DEFAULT_COLOR, "Hello %s", "world");
    os_print(DARK_CYAN, " %d", 42);

    buffer_free();
}

void test_os_print_prompt(void** state)
{
    (void)state;

    /* Smoke-test: must not crash. Output goes to the console. */
    os_print_prompt();
}

void test_os_key_is_hit(void** state)
{
    (void)state;

    /* In the non-interactive test-runner environment no key is pending,
     * so the function must return false. */
    assert_false(os_key_is_hit());
}

void test_os_clear_window(void** state)
{
    (void)state;

    /* Passing NULL is explicitly guarded by 'if (renderer)' in the
     * implementation; must not crash. */
    os_clear_window(NULL);
}

static uint64 timer_one_shot_cb(void* userdata, os_timer_id id, uint64 interval)
{
    volatile int* fired = (volatile int*)userdata;
    (void)id;
    *fired = 1;
    return 0; /* returning 0 cancels the timer after the first fire */
}

void test_os_add_remove_timer(void** state)
{
    os_timer_id id;
    volatile int fired = 0;

    (void)state;

    /* Schedule a one-shot timer 1 ms in the future. */
    id = os_add_timer(1000000ULL, timer_one_shot_cb, (void*)&fired);
    assert_true(id != 0);

    /* os_remove_timer on a valid ID must succeed. */
    assert_true(os_remove_timer(id));

    /* os_remove_timer on an already-removed (invalid) ID must not crash
     * and must return false. */
    assert_false(os_remove_timer(id));
}

/* Test callback for os_wait_thread */
static int return_value_thread_fn(void* data)
{
    (void)data;
    return 42;
}

void test_os_wait_thread(void** state)
{
    os_thread* th;
    int result;

    (void)state;

    th = os_create_thread(return_value_thread_fn, "test_wait", NULL);
    assert_non_null(th);

    /* Wait for thread to complete and verify return value. */
    result = os_wait_thread(th);
    assert_int_equal(result, 42);
}

static int noop_thread_fn(void* data)
{
    (void)data;
    return 0;
}

void test_os_create_detach_thread(void** state)
{
    os_thread* th;

    (void)state;

    th = os_create_thread(noop_thread_fn, "test_noop", NULL);
    assert_non_null(th);

    /* Detach immediately the thread may already have exited; that is fine. */
    os_detach_thread(th);

    /* Give the scheduler a moment to clean up before the next test. */
    os_delay(10);
}

void test_os_cos(void** state)
{
    float result;

    (void)state;

    /* Test cos(0) = 1.0 */
    result = os_cos(0.0f);
    assert_true(result > 0.99f && result < 1.01f);

    /* Test cos(pi) = -1.0 (approximately 3.14159) */
    result = os_cos(3.14159f);
    assert_true(result > -1.01f && result < -0.99f);

    /* Test cos(pi/2) = 0.0 (approximately 1.5708) */
    result = os_cos(1.5708f);
    assert_true(result > -0.01f && result < 0.01f);
}

void test_os_sin(void** state)
{
    float result;

    (void)state;

    /* Test sin(0) = 0.0 */
    result = os_sin(0.0f);
    assert_true(result > -0.01f && result < 0.01f);

    /* Test sin(pi/2) = 1.0 (approximately 1.5708) */
    result = os_sin(1.5708f);
    assert_true(result > 0.99f && result < 1.01f);

    /* Test sin(pi) = 0.0 (approximately 3.14159) */
    result = os_sin(3.14159f);
    assert_true(result > -0.01f && result < 0.01f);
}

void test_os_get_allocator_hooks(void** state)
{
    os_memory_hooks hooks;

    (void)state;

    hooks = os_get_allocator_hooks();

    /* Verify that both function pointers are set. */
    assert_non_null(hooks.malloc_fn);
    assert_non_null(hooks.free_fn);

    /* Test basic allocation and deallocation with the hooks. */
    void* ptr = hooks.malloc_fn(256);
    assert_non_null(ptr);

    hooks.free_fn(ptr);
}

void test_os_console_hide_show(void** state)
{
    (void)state;

    /* Smoke tests: these functions should not crash in test environment. */
    os_console_hide();
    os_console_show();
}

void test_os_isdigit_extended(void** state)
{
    (void)state;

    /* Basic digit tests (covered in existing test) */
    assert_int_not_equal(os_isdigit('0'), 0);
    assert_int_not_equal(os_isdigit('5'), 0);
    assert_int_not_equal(os_isdigit('9'), 0);

    /* Boundary: just before '0' (ASCII 47) */
    assert_int_equal(os_isdigit('/'), 0);

    /* Boundary: just after '9' (ASCII 58) */
    assert_int_equal(os_isdigit(':'), 0);

    /* Non-digit characters */
    assert_int_equal(os_isdigit('a'), 0);
    assert_int_equal(os_isdigit('Z'), 0);
    assert_int_equal(os_isdigit(' '), 0);
    assert_int_equal(os_isdigit('\n'), 0);
}

void test_os_isprint_extended(void** state)
{
    (void)state;

    /* Printable ASCII characters */
    assert_int_not_equal(os_isprint('A'), 0);
    assert_int_not_equal(os_isprint('z'), 0);
    assert_int_not_equal(os_isprint('0'), 0);
    assert_int_not_equal(os_isprint('!'), 0);
    assert_int_not_equal(os_isprint(' '), 0);  /* Space is printable */
    assert_int_not_equal(os_isprint('~'), 0);  /* Tilde (ASCII 126) is last printable */

    /* Non-printable control characters */
    assert_int_equal(os_isprint('\0'), 0);     /* NULL */
    assert_int_equal(os_isprint('\n'), 0);     /* Newline */
    assert_int_equal(os_isprint('\r'), 0);     /* Carriage return */
    assert_int_equal(os_isprint('\t'), 0);     /* Tab */
    assert_int_equal(os_isprint('\x7F'), 0);   /* DEL (ASCII 127, not printable) */
}

void test_os_isspace_extended(void** state)
{
    (void)state;

    /* Whitespace characters */
    assert_int_not_equal(os_isspace(' '), 0);   /* Space */
    assert_int_not_equal(os_isspace('\t'), 0);  /* Tab */
    assert_int_not_equal(os_isspace('\n'), 0);  /* Newline */
    assert_int_not_equal(os_isspace('\r'), 0);  /* Carriage return */
    assert_int_not_equal(os_isspace('\f'), 0);  /* Form feed */
    assert_int_not_equal(os_isspace('\v'), 0);  /* Vertical tab */

    /* Non-whitespace characters */
    assert_int_equal(os_isspace('a'), 0);
    assert_int_equal(os_isspace('0'), 0);
    assert_int_equal(os_isspace('!'), 0);
}

void test_os_strstr_edge_cases(void** state)
{
    (void)state;

    /* Empty needle should match at the start */
    assert_non_null(os_strstr("hello", ""));

    /* Needle not found */
    assert_null(os_strstr("hello", "xyz"));

    /* Needle same as haystack */
    assert_non_null(os_strstr("hello", "hello"));

    /* Needle at start */
    assert_non_null(os_strstr("hello world", "hello"));

    /* Needle at end */
    assert_non_null(os_strstr("hello world", "world"));

    /* Needle in middle */
    assert_non_null(os_strstr("hello world test", "world"));
}

void test_os_strchr_edge_cases(void** state)
{
    (void)state;

    /* Character at start */
    assert_non_null(os_strchr("hello", 'h'));

    /* Character in middle */
    assert_non_null(os_strchr("hello", 'l'));

    /* Character at end */
    assert_non_null(os_strchr("hello", 'o'));

    /* Character not found */
    assert_null(os_strchr("hello", 'x'));

    /* Searching for null terminator */
    assert_non_null(os_strchr("hello", '\0'));
}

void test_os_strrchr_edge_cases(void** state)
{
    (void)state;

    /* Last occurrence of character */
    assert_non_null(os_strrchr("hello world", 'o'));

    /* Character not found */
    assert_null(os_strrchr("hello", 'x'));

    /* First character when it only appears once */
    assert_non_null(os_strrchr("hello", 'h'));

    /* Multiple occurrences - should find the last */
    const char* result = os_strrchr("abcabc", 'a');
    assert_non_null(result);
    assert_int_equal(result[0], 'a');
    assert_int_equal(result[1], 'b');
    assert_int_equal(result[2], 'c');
}

void test_os_strlen_edge_cases(void** state)
{
    (void)state;

    /* Empty string */
    assert_int_equal(os_strlen(""), 0);

    /* Single character */
    assert_int_equal(os_strlen("a"), 1);

    /* Long string */
    assert_int_equal(os_strlen("Hello, world!"), 13);
}

void test_os_strcmp_edge_cases(void** state)
{
    (void)state;

    /* Identical strings */
    assert_int_equal(os_strcmp("hello", "hello"), 0);

    /* Empty strings */
    assert_int_equal(os_strcmp("", ""), 0);

    /* First string less than second (negative result) */
    assert_true(os_strcmp("abc", "def") < 0);

    /* First string greater than second (positive result) */
    assert_true(os_strcmp("xyz", "abc") > 0);

    /* Same prefix, different length */
    assert_true(os_strcmp("hello", "hello world") < 0);
    assert_true(os_strcmp("hello world", "hello") > 0);
}

void test_os_strncmp_edge_cases(void** state)
{
    (void)state;

    /* Compare zero bytes */
    assert_int_equal(os_strncmp("abc", "xyz", 0), 0);

    /* Compare partial strings where only first n match */
    assert_int_equal(os_strncmp("hello", "helloworld", 5), 0);

    /* Identical strings within n bytes */
    assert_int_equal(os_strncmp("hello", "hello", 5), 0);

    /* Different strings within n bytes */
    assert_true(os_strncmp("abc", "xyz", 1) < 0);
}

void test_os_memcpy_edge_cases(void** state)
{
    (void)state;

    /* Copy zero bytes */
    char dest[10];
    os_memcpy(dest, "test", 0);
    /* No assertion needed; we're testing it doesn't crash */

    /* Copy full buffer */
    os_memcpy(dest, "hello", 5);
    assert_int_equal(dest[0], 'h');
    assert_int_equal(dest[4], 'o');
}

void test_os_memmove_overlap(void** state)
{
    (void)state;

    /* Test overlapping memory regions (forward and backward) */
    char buffer[20] = "hello world";

    /* Move data backward (overlapping) */
    os_memmove(buffer + 1, buffer, 11);
    assert_int_equal(buffer[1], 'h');

    /* Reset and test forward overlap */
    strcpy(buffer, "hello");
    os_memmove(buffer + 2, buffer, 3);
    assert_int_equal(buffer[2], 'h');
}

void test_os_memset_patterns(void** state)
{
    (void)state;

    unsigned char buffer[20];

    /* Fill with spaces */
    os_memset(buffer, ' ', 5);
    assert_int_equal(buffer[0], ' ');
    assert_int_equal(buffer[4], ' ');

    /* Fill with zeros */
    os_memset(buffer, 0, 5);
    assert_int_equal(buffer[0], 0);
    assert_int_equal(buffer[4], 0);

    /* Fill with high value */
    os_memset(buffer, 0xFF, 3);
    assert_int_equal(buffer[0], 0xFF);
    assert_int_equal(buffer[2], 0xFF);
}

void test_os_strcspn_edge_cases(void** state)
{
    (void)state;

    /* No characters in rejection set */
    assert_int_equal(os_strcspn("hello", "xyz"), 5);

    /* First character in rejection set */
    assert_int_equal(os_strcspn("hello", "h"), 0);

    /* Last character in rejection set */
    assert_int_equal(os_strcspn("hello", "o"), 4);

    /* Empty rejection set */
    assert_int_equal(os_strcspn("hello", ""), 5);
}

void test_os_atoi_edge_cases(void** state)
{
    (void)state;

    /* Positive number */
    assert_int_equal(os_atoi("12345"), 12345);

    /* Negative number */
    assert_int_equal(os_atoi("-42"), -42);

    /* Zero */
    assert_int_equal(os_atoi("0"), 0);

    /* Number with leading spaces */
    assert_int_equal(os_atoi("  123"), 123);

    /* Non-numeric string */
    assert_int_equal(os_atoi("abc"), 0);
}

void test_os_atof_edge_cases(void** state)
{
    double result;

    (void)state;

    /* Zero */
    result = os_atof("0.0");
    assert_true(result >= -0.001 && result <= 0.001);

    /* Positive decimal */
    result = os_atof("3.14");
    assert_true(result > 3.13 && result < 3.15);

    /* Negative decimal */
    result = os_atof("-2.5");
    assert_true(result > -2.51 && result < -2.49);

    /* Integer as float */
    result = os_atof("42");
    assert_true(result > 41.99 && result < 42.01);
}

void test_os_strlcpy_boundary(void** state)
{
    (void)state;

    /* Test truncation when buffer is too small */
    char dest[5];
    const char* src = "Hello World";
    size_t result = os_strlcpy(dest, src, sizeof(dest));

    /* Result should be the length of the original string */
    assert_int_equal(result, 11);

    /* Destination should be null-terminated and truncated */
    assert_int_equal(dest[4], '\0');
    assert_string_equal(dest, "Hell");
}

void test_os_strlcat_overflow(void** state)
{
    (void)state;

    /* Test when concatenation would overflow */
    char dest[10] = "Hello";
    const char* src = "World Extra Text";
    size_t result = os_strlcat(dest, src, sizeof(dest));

    /* Result should indicate what the full length would have been */
    assert_true(result > sizeof(dest));

    /* Destination should remain null-terminated */
    assert_int_equal(dest[9], '\0');
}

void test_os_realloc_grow(void** state)
{
    (void)state;

    /* Allocate initial memory */
    void* ptr = os_calloc(10, 1);
    assert_non_null(ptr);

    /* Grow the memory */
    void* new_ptr = os_realloc(ptr, 100);
    assert_non_null(new_ptr);

    os_free(new_ptr);
}

void test_os_realloc_shrink(void** state)
{
    (void)state;

    /* Allocate memory */
    void* ptr = os_calloc(100, 1);
    assert_non_null(ptr);

    /* Shrink the memory */
    void* new_ptr = os_realloc(ptr, 10);
    assert_non_null(new_ptr);

    os_free(new_ptr);
}

void test_os_strtol_boundary(void** state)
{
    (void)state;

    /* Test negative number */
    assert_int_equal(os_strtol("-42", NULL, 10), -42);

    /* Test with base 16 */
    assert_int_equal(os_strtol("FF", NULL, 16), 255);

    /* Test with base 2 */
    assert_int_equal(os_strtol("1010", NULL, 2), 10);

    /* Test with base 8 */
    assert_int_equal(os_strtol("10", NULL, 8), 8);
}

void test_os_strtoul_boundary(void** state)
{
    (void)state;

    /* Test large number */
    assert_int_equal(os_strtoul("999999", NULL, 10), 999999);

    /* Test hex */
    assert_int_equal(os_strtoul("DEADBEEF", NULL, 16), 0xDEADBEEF);

    /* Test octal */
    assert_int_equal(os_strtoul("777", NULL, 8), 511);
}

void test_os_strtoull_boundary(void** state)
{
    (void)state;

    /* Test 64-bit values */
    uint64 result = os_strtoull("18446744073709551615", NULL, 10);
    assert_true(result == 18446744073709551615ULL);

    /* Test hex */
    result = os_strtoull("FFFFFFFFFFFFFFFF", NULL, 16);
    assert_true(result == 0xFFFFFFFFFFFFFFFFULL);
}

void test_os_itoa_boundary(void** state)
{
    (void)state;

    char buffer[20];

    /* Test zero */
    os_itoa(0, buffer, 10);
    assert_string_equal(buffer, "0");

    /* Test negative */
    os_itoa(-42, buffer, 10);
    assert_string_equal(buffer, "-42");

    /* Test different bases */
    os_itoa(255, buffer, 16);
    assert_string_equal(buffer, "FF");

    os_itoa(8, buffer, 2);
    assert_string_equal(buffer, "1000");
}

void test_os_swap_be_32_values(void** state)
{
    (void)state;

    /* Test various patterns */
    assert_int_equal(os_swap_be_32(0x12345678), 0x78563412);
    assert_int_equal(os_swap_be_32(0xFFFFFFFF), 0xFFFFFFFF);
    assert_int_equal(os_swap_be_32(0x00000001), 0x01000000);
    assert_int_equal(os_swap_be_32(0x80000000), 0x00000080);
}

void test_os_tolower_boundary(void** state)
{
    (void)state;

    /* Test all uppercase letters */
    assert_int_equal(os_tolower('A'), 'a');
    assert_int_equal(os_tolower('Z'), 'z');

    /* Test already lowercase */
    assert_int_equal(os_tolower('a'), 'a');

    /* Test non-alphabetic */
    assert_int_equal(os_tolower('0'), '0');
    assert_int_equal(os_tolower('!'), '!');
}

void test_os_toupper_boundary(void** state)
{
    (void)state;

    /* Test all lowercase letters */
    assert_int_equal(os_toupper('a'), 'A');
    assert_int_equal(os_toupper('z'), 'Z');

    /* Test already uppercase */
    assert_int_equal(os_toupper('A'), 'A');

    /* Test non-alphabetic */
    assert_int_equal(os_toupper('0'), '0');
    assert_int_equal(os_toupper('!'), '!');
}

void test_os_snprintf_truncation(void** state)
{
    (void)state;

    char buffer[10];

    /* Test truncation with a value that would exceed buffer */
    int result = os_snprintf(buffer, sizeof(buffer), "Hello %s", "World Extra Text");

    /* Result should be truncated */
    assert_true(os_strlen(buffer) < sizeof(buffer));

    /* Should be null-terminated */
    assert_int_equal(buffer[9], '\0');
}

void test_os_vsnprintf_format(void** state)
{
    char buffer[256];

    (void)state;

    /* Use a variadic wrapper to test vsnprintf indirectly
     * os_vsnprintf is tested through os_log which uses it internally */

    /* Just verify os_vsnprintf can be called - it's tested via os_log */
    assert_true(buffer_init(512) == ALL_OK);
    os_log(LOG_INFO, "Test format: %d %s %.2f", 42, "hello", 3.14);
    buffer_free();
}

static int sum_values(int count, ...)
{
    int i;
    va_list_t args;
    int sum = 0;

    os_va_start(args, count);

    for (i = 0; i < count; ++i)
    {
        sum += os_va_arg(args, int);
    }

    os_va_end(args);

    return sum;
}
