/** @file tefst_scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define _mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define _mkdir(dir, mode) mkdir(dir, mode)
#endif

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cmocka.h"
#include "core.h"
#include "os.h"
#include "scripts.h"
#include "test_scripts.h"

static int stdout_fd_backup;
static int stdout_fd;

static status_t test_python_script(const char* file_name);

void test_has_valid_extension(void** state)
{
    (void)state;

    assert_true(has_valid_extension("exceptional_script.lua") == IS_TRUE);
    assert_true(has_valid_extension("mediocre_script.py") == IS_TRUE);
    assert_true(has_valid_extension("totally_unsupported.tcl") == IS_FALSE);
}

void test_lua(void** state)
{
    core_t  core = {0};
    FILE_t* lua_file;

    (void)state;

    if (_mkdir("tests", 0777) != 0)
    {
        if (errno != EEXIST)
        {
            os_fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
            return;
        }
    }

    lua_file = os_fopen("tests/test.lua", "w+");

    if (lua_file == NULL)
    {
        os_fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return;
    }

    scripts_init(&core);

    os_fprintf(lua_file, "-- Lua script to test and validate basic Lua functionality\n\n");

    /* Basic Lua operations. */
    os_fprintf(lua_file, "-- Variable assignment\n");
    os_fprintf(lua_file, "local x = 10\n");

    os_fprintf(lua_file, "-- Arithmetic operation\n");
    os_fprintf(lua_file, "local y = x + 5\n");

    os_fprintf(lua_file, "-- String concatenation\n");
    os_fprintf(lua_file, "local message = 'Lua is great!'\n");

    os_fprintf(lua_file, "-- Table creation and indexing\n");
    os_fprintf(lua_file, "local t = { 'apple', 'banana', 'cherry' }\n");
    os_fprintf(lua_file, "local fruit = t[2]\n");

    os_fprintf(lua_file, "-- Function definition\n");
    os_fprintf(lua_file, "local function add(a, b)\n");
    os_fprintf(lua_file, "    return a + b\n");
    os_fprintf(lua_file, "end\n");

    os_fprintf(lua_file, "-- Function call\n");
    os_fprintf(lua_file, "local result = add(x, y)\n");

    os_fprintf(lua_file, "-- Boolean comparison\n");
    os_fprintf(lua_file, "local isGreater = x > y\n");

    os_fprintf(lua_file, "-- Control structure (if-else)\n");
    os_fprintf(lua_file, "if isGreater then\n");
    os_fprintf(lua_file, "    result = result * 2\n");
    os_fprintf(lua_file, "else\n");
    os_fprintf(lua_file, "    result = result / 2\n");
    os_fprintf(lua_file, "end\n");

    /* Assertions to validate results. */
    os_fprintf(lua_file, "-- Assertions to validate results\n");
    os_fprintf(lua_file, "assert(x == 10, 'Variable x should be 10')\n");
    os_fprintf(lua_file, "assert(y == 15, 'Variable y should be 15')\n");
    os_fprintf(lua_file, "assert(message == 'Lua is great!', 'Message should be Lua is great!')\n");
    os_fprintf(lua_file, "assert(fruit == 'banana', 'Fruit should be banana')\n");
    os_fprintf(lua_file, "assert(result == 12.5, 'Result should be 12.5')\n");

    os_fclose(lua_file);
    assert_true(run_script_ex("tests/test.lua", &core) == ALL_OK);

    scripts_deinit(&core);
}

void test_python_01_int(void** state)
{
    assert_true(test_python_script("01_int.py") == ALL_OK);
}

void test_python_02_float(void** state)
{
    assert_true(test_python_script("02_float.py") == ALL_OK);
}

void test_python_03_bool(void** state)
{
    assert_true(test_python_script("03_bool.py") == ALL_OK);
}

void test_python_04_line_continue(void** state)
{
    assert_true(test_python_script("04_line_continue.py") == ALL_OK);
}

void test_python_04_str(void** state)
{
    assert_true(test_python_script("04_str.py") == ALL_OK);
}

void test_python_05_list(void** state)
{
    assert_true(test_python_script("05_list.py") == ALL_OK);
}

void test_python_06_tuple(void** state)
{
    assert_true(test_python_script("06_tuple.py") == ALL_OK);
}

void test_python_07_listcomp(void** state)
{
    assert_true(test_python_script("07_listcomp.py") == ALL_OK);
}

void test_python_08_dictcomp(void** state)
{
    assert_true(test_python_script("08_dictcomp.py") == ALL_OK);
}

void test_python_08_dict(void** state)
{
    assert_true(test_python_script("08_dict.py") == ALL_OK);
}

void test_python_15_assign(void** state)
{
    assert_true(test_python_script("15_assign.py") == ALL_OK);
}

void test_python_15_cmp(void** state)
{
    assert_true(test_python_script("15_cmp.py") == ALL_OK);
}

void test_python_15_controlflow(void** state)
{
    assert_true(test_python_script("15_controlflow.py") == ALL_OK);
}

void test_python_16_functions(void** state)
{
    assert_true(test_python_script("16_functions.py") == ALL_OK);
}

void test_python_16_typehints(void** state)
{
    assert_true(test_python_script("16_typehints.py") == ALL_OK);
}

void test_python_24_inline_blocks(void** state)
{
    assert_true(test_python_script("24_inline_blocks.py") == ALL_OK);
}

void test_python_25_rfstring(void** state)
{
    assert_true(test_python_script("25_rfstring.py") == ALL_OK);
}

void test_python_26_multiline(void** state)
{
    assert_true(test_python_script("26_multiline.py") == ALL_OK);
}

void test_python_28_exception(void** state)
{
    assert_true(test_python_script("28_exception.py") == ALL_OK);
}

void test_python_29_iter(void** state)
{
    assert_true(test_python_script("29_iter.py") == ALL_OK);
}

void test_python_30_import(void** state)
{
    assert_true(test_python_script("30_import.py") == ALL_OK);
}

void test_python_40_class(void** state)
{
    assert_true(test_python_script("40_class.py") == ALL_OK);
}

void test_python_41_class_ex(void** state)
{
    assert_true(test_python_script("41_class_ex.py") == ALL_OK);
}

void test_python_42_decorator(void** state)
{
    assert_true(test_python_script("42_decorator.py") == ALL_OK);
}

void test_python_43_closure(void** state)
{
    assert_true(test_python_script("43_closure.py") == ALL_OK);
}

void test_python_44_star(void** state)
{
    assert_true(test_python_script("44_star.py") == ALL_OK);
}

void test_python_46_bytes(void** state)
{
    assert_true(test_python_script("46_bytes.py") == ALL_OK);
}

void test_python_47_set(void** state)
{
    assert_true(test_python_script("47_set.py") == ALL_OK);
}

void test_python_48_setcomp(void** state)
{
    assert_true(test_python_script("48_setcomp.py") == ALL_OK);
}

void test_python_50_reflection(void** state)
{
    assert_true(test_python_script("50_reflection.py") == ALL_OK);
}

void test_python_51_yield(void** state)
{
    assert_true(test_python_script("51_yield.py") == ALL_OK);
}

void test_python_52_context(void** state)
{
    assert_true(test_python_script("52_context.py") == ALL_OK);
}

void test_python_66_eval(void** state)
{
    assert_true(test_python_script("66_eval.py") == ALL_OK);
}

void test_python_70_bisect(void** state)
{
    assert_true(test_python_script("70_bisect.py") == ALL_OK);
}

void test_python_70_builtins(void** state)
{
    assert_true(test_python_script("70_builtins.py") == ALL_OK);
}

void test_python_70_heapq(void** state)
{
    assert_true(test_python_script("70_heapq.py") == ALL_OK);
}

void test_python_70_math(void** state)
{
    assert_true(test_python_script("70_math.py") == ALL_OK);
}

void test_python_70_random(void** state)
{
    assert_true(test_python_script("70_random.py") == ALL_OK);
}

void test_python_71_cmath(void** state)
{
    assert_true(test_python_script("71_cmath.py") == ALL_OK);
}

void test_python_71_gc(void** state)
{
    assert_true(test_python_script("71_gc.py") == ALL_OK);
}

void test_python_72_collections(void** state)
{
    assert_true(test_python_script("72_collections.py") == ALL_OK);
}

void test_python_72_json(void** state)
{
    assert_true(test_python_script("72_json.py") == ALL_OK);
}

void test_python_73_functools(void** state)
{
    assert_true(test_python_script("73_functools.py") == ALL_OK);
}

void test_python_73_json_alt(void** state)
{
    assert_true(test_python_script("73_json_alt.py") == ALL_OK);
}

void test_python_73_typing(void** state)
{
    assert_true(test_python_script("73_typing.py") == ALL_OK);
}

void test_python_74_operator(void** state)
{
    assert_true(test_python_script("74_operator.py") == ALL_OK);
}

void test_python_75_compile(void** state)
{
    assert_true(test_python_script("75_compile.py") == ALL_OK);
}

void test_python_76_dna(void** state)
{
    assert_true(test_python_script("76_dna.py") == ALL_OK);
}

void test_python_76_misc(void** state)
{
    assert_true(test_python_script("76_misc.py") == ALL_OK);
}

void test_python_76_prime(void** state)
{
    assert_true(test_python_script("76_prime.py") == ALL_OK);
}

void test_python_77_builtin_func(void** state)
{
    assert_true(test_python_script("77_builtin_func.py") == ALL_OK);
}

void test_python_79_datetime(void** state)
{
    assert_true(test_python_script("79_datetime.py") == ALL_OK);
}

void test_python_79_easing(void** state)
{
    assert_true(test_python_script("79_easing.py") == ALL_OK);
}

void test_python_79_file(void** state)
{
    assert_true(test_python_script("79_file.py") == ALL_OK);
}

void test_python_80_linalg(void** state)
{
    assert_true(test_python_script("80_linalg.py") == ALL_OK);
}

void test_python_80_sys(void** state)
{
    assert_true(test_python_script("80_sys.py") == ALL_OK);
}

void test_python_80_traceback(void** state)
{
    assert_true(test_python_script("80_traceback.py") == ALL_OK);
}

void test_python_81_dataclasses(void** state)
{
    assert_true(test_python_script("81_dataclasses.py") == ALL_OK);
}

void test_python_82_enum(void** state)
{
    assert_true(test_python_script("82_enum.py") == ALL_OK);
}

void test_python_90_array2d(void** state)
{
    assert_true(test_python_script("90_array2d.py") == ALL_OK);
}

void test_python_90_pickle(void** state)
{
    assert_true(test_python_script("90_pickle.py") == ALL_OK);
}

void test_python_91_line_profiler(void** state)
{
    assert_true(test_python_script("91_line_profiler.py") == ALL_OK);
}

void test_python_91_pdb(void** state)
{
    assert_true(test_python_script("91_pdb.py") == ALL_OK);
}

void test_python_95_bugs(void** state)
{
    assert_true(test_python_script("95_bugs.py") == ALL_OK);
}

void test_python_95_dis(void** state)
{
    assert_true(test_python_script("95_dis.py") == ALL_OK);
}

void test_python_96_pep695_py312(void** state)
{
    assert_true(test_python_script("96_pep695_py312.py") == ALL_OK);
}

void test_python_99_extras(void** state)
{
    assert_true(test_python_script("99_extras.py") == ALL_OK);
}

static status_t test_python_script(const char* script_name)
{
    status_t status;
    core_t   core              = {0};
    char     script_path[1024] = {0};

    os_snprintf(script_path, sizeof(script_path), "tests/%s", script_name);

    scripts_init(&core);
    status = run_script_ex(script_path, &core);
    scripts_deinit(&core);

    return status;
}
