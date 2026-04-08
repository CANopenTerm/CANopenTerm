/** @file test_scripts.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
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
static core_t test_core = {0};

void test_has_valid_extension(void** state)
{
    (void)state;

    assert_true(has_valid_extension("exceptional_script.lua") == true);
    assert_true(has_valid_extension("mediocre_script.py") == true);
    assert_true(has_valid_extension("totally_unsupported.tcl") == false);
}

void test_lua(void** state)
{
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

    scripts_init(&test_core);

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
    assert_true(run_script_ex("tests/test.lua", &test_core) == ALL_OK);
}

void test_python_010_int(void** state)
{
    assert_true(test_python_script("010_int.py") == ALL_OK);
}

void test_python_020_float(void** state)
{
    assert_true(test_python_script("020_float.py") == ALL_OK);
}

void test_python_030_bool(void** state)
{
    assert_true(test_python_script("030_bool.py") == ALL_OK);
}

void test_python_040_line_continue(void** state)
{
    assert_true(test_python_script("040_line_continue.py") == ALL_OK);
}

void test_python_041_str(void** state)
{
    assert_true(test_python_script("041_str.py") == ALL_OK);
}

void test_python_042_str_mod(void** state)
{
    assert_true(test_python_script("042_str_mod.py") == ALL_OK);
}

void test_python_050_list(void** state)
{
    assert_true(test_python_script("050_list.py") == ALL_OK);
}

void test_python_060_tuple(void** state)
{
    assert_true(test_python_script("060_tuple.py") == ALL_OK);
}

void test_python_070_listcomp(void** state)
{
    assert_true(test_python_script("070_listcomp.py") == ALL_OK);
}

void test_python_080_dict(void** state)
{
    assert_true(test_python_script("080_dict.py") == ALL_OK);
}

void test_python_081_dictcomp(void** state)
{
    assert_true(test_python_script("081_dictcomp.py") == ALL_OK);
}

void test_python_150_assign(void** state)
{
    assert_true(test_python_script("150_assign.py") == ALL_OK);
}

void test_python_151_cmp(void** state)
{
    assert_true(test_python_script("151_cmp.py") == ALL_OK);
}

void test_python_152_controlflow(void** state)
{
    assert_true(test_python_script("152_controlflow.py") == ALL_OK);
}

void test_python_160_functions(void** state)
{
    assert_true(test_python_script("160_functions.py") == ALL_OK);
}

void test_python_161_typehints(void** state)
{
    assert_true(test_python_script("161_typehints.py") == ALL_OK);
}

void test_python_240_inline_blocks(void** state)
{
    assert_true(test_python_script("240_inline_blocks.py") == ALL_OK);
}

void test_python_250_rfstring(void** state)
{
    assert_true(test_python_script("250_rfstring.py") == ALL_OK);
}

void test_python_260_multiline(void** state)
{
    assert_true(test_python_script("260_multiline.py") == ALL_OK);
}

void test_python_280_exception(void** state)
{
    assert_true(test_python_script("280_exception.py") == ALL_OK);
}

void test_python_290_iter(void** state)
{
    assert_true(test_python_script("290_iter.py") == ALL_OK);
}

void test_python_300_import(void** state)
{
    assert_true(test_python_script("300_import.py") == ALL_OK);
}

void test_python_301_import1(void** state)
{
    assert_true(test_python_script("301_import1.py") == ALL_OK);
}

void test_python_310_modulereload(void** state)
{
    assert_true(test_python_script("310_modulereload.py") == ALL_OK);
}

void test_python_400_class(void** state)
{
    assert_true(test_python_script("400_class.py") == ALL_OK);
}

void test_python_410_class_ex(void** state)
{
    assert_true(test_python_script("410_class_ex.py") == ALL_OK);
}

void test_python_420_decorator(void** state)
{
    assert_true(test_python_script("420_decorator.py") == ALL_OK);
}

void test_python_430_closure(void** state)
{
    assert_true(test_python_script("430_closure.py") == ALL_OK);
}

void test_python_440_star(void** state)
{
    assert_true(test_python_script("440_star.py") == ALL_OK);
}

void test_python_460_bytes(void** state)
{
    assert_true(test_python_script("460_bytes.py") == ALL_OK);
}

void test_python_470_set(void** state)
{
    assert_true(test_python_script("470_set.py") == ALL_OK);
}

void test_python_480_setcomp(void** state)
{
    assert_true(test_python_script("480_setcomp.py") == ALL_OK);
}

void test_python_500_reflection(void** state)
{
    assert_true(test_python_script("500_reflection.py") == ALL_OK);
}

void test_python_510_yield(void** state)
{
    assert_true(test_python_script("510_yield.py") == ALL_OK);
}

void test_python_520_context(void** state)
{
    assert_true(test_python_script("520_context.py") == ALL_OK);
}

void test_python_660_eval(void** state)
{
    assert_true(test_python_script("660_eval.py") == ALL_OK);
}

void test_python_661_exec_bug(void** state)
{
    assert_true(test_python_script("661_exec_bug.py") == ALL_OK);
}

void test_python_670_locals_vs_globals(void** state)
{
    assert_true(test_python_script("670_locals_vs_globals.py") == ALL_OK);
}

void test_python_700_base64(void** state)
{
    assert_true(test_python_script("700_base64.py") == ALL_OK);
}

void test_python_701_bisect(void** state)
{
    assert_true(test_python_script("701_bisect.py") == ALL_OK);
}

void test_python_702_builtins(void** state)
{
    assert_true(test_python_script("702_builtins.py") == ALL_OK);
}

void test_python_703_heapq(void** state)
{
    assert_true(test_python_script("703_heapq.py") == ALL_OK);
}

void test_python_704_math(void** state)
{
    assert_true(test_python_script("704_math.py") == ALL_OK);
}

void test_python_705_random(void** state)
{
    assert_true(test_python_script("705_random.py") == ALL_OK);
}

void test_python_710_cmath(void** state)
{
    assert_true(test_python_script("710_cmath.py") == ALL_OK);
}

void test_python_711_gc(void** state)
{
    assert_true(test_python_script("711_gc.py") == ALL_OK);
}

void test_python_720_collections(void** state)
{
    assert_true(test_python_script("720_collections.py") == ALL_OK);
}

void test_python_721_json(void** state)
{
    assert_true(test_python_script("721_json.py") == ALL_OK);
}

void test_python_722_lz4(void** state)
{
    assert_true(test_python_script("722_lz4.py") == ALL_OK);
}

void test_python_723_msgpack(void** state)
{
    assert_true(test_python_script("723_msgpack.py") == ALL_OK);
}

void test_python_730_functools(void** state)
{
    assert_true(test_python_script("730_functools.py") == ALL_OK);
}

void test_python_731_json_alt(void** state)
{
    assert_true(test_python_script("731_json_alt.py") == ALL_OK);
}

void test_python_732_json_indent(void** state)
{
    assert_true(test_python_script("732_json_indent.py") == ALL_OK);
}

void test_python_733_typing(void** state)
{
    assert_true(test_python_script("733_typing.py") == ALL_OK);
}

void test_python_740_operator(void** state)
{
    assert_true(test_python_script("740_operator.py") == ALL_OK);
}

void test_python_750_compile(void** state)
{
    assert_true(test_python_script("750_compile.py") == ALL_OK);
}

void test_python_760_dna(void** state)
{
    assert_true(test_python_script("760_dna.py") == ALL_OK);
}

void test_python_761_misc(void** state)
{
    assert_true(test_python_script("761_misc.py") == ALL_OK);
}

void test_python_762_prime(void** state)
{
    assert_true(test_python_script("762_prime.py") == ALL_OK);
}

void test_python_770_builtin_func_1(void** state)
{
    assert_true(test_python_script("770_builtin_func_1.py") == ALL_OK);
}

void test_python_771_builtin_func_2(void** state)
{
    assert_true(test_python_script("771_builtin_func_2.py") == ALL_OK);
}

void test_python_790_datetime(void** state)
{
    assert_true(test_python_script("790_datetime.py") == ALL_OK);
}

void test_python_791_easing(void** state)
{
    assert_true(test_python_script("791_easing.py") == ALL_OK);
}

void test_python_792_file(void** state)
{
    assert_true(test_python_script("792_file.py") == ALL_OK);
}

void test_python_793_stdc(void** state)
{
    assert_true(test_python_script("793_stdc.py") == ALL_OK);
}

void test_python_800_color32(void** state)
{
    assert_true(test_python_script("800_color32.py") == ALL_OK);
}

void test_python_801_sys(void** state)
{
    assert_true(test_python_script("801_sys.py") == ALL_OK);
}

void test_python_802_traceback(void** state)
{
    assert_true(test_python_script("802_traceback.py") == ALL_OK);
}

void test_python_803_vmath(void** state)
{
    assert_true(test_python_script("803_vmath.py") == ALL_OK);
}

void test_python_810_dataclasses(void** state)
{
    assert_true(test_python_script("810_dataclasses.py") == ALL_OK);
}

void test_python_820_enum(void** state)
{
    assert_true(test_python_script("820_enum.py") == ALL_OK);
}

void test_python_830_unicodedata(void** state)
{
    assert_true(test_python_script("830_unicodedata.py") == ALL_OK);
}

void test_python_900_array2d(void** state)
{
    assert_true(test_python_script("900_array2d.py") == ALL_OK);
}

void test_python_901_array2d_extra1(void** state)
{
    assert_true(test_python_script("901_array2d_extra1.py") == ALL_OK);
}

void test_python_902_chunked_array2d(void** state)
{
    assert_true(test_python_script("902_chunked_array2d.py") == ALL_OK);
}

void test_python_903_colorcvt(void** state)
{
    assert_true(test_python_script("903_colorcvt.py") == ALL_OK);
}

void test_python_904_pickle(void** state)
{
    assert_true(test_python_script("904_pickle.py") == ALL_OK);
}

void test_python_910_line_profiler(void** state)
{
    assert_true(test_python_script("910_line_profiler.py") == ALL_OK);
}

void test_python_920_picoterm(void** state)
{
    assert_true(test_python_script("920_picoterm.py") == ALL_OK);
}

void test_python_921_pkpy(void** state)
{
    assert_true(test_python_script("921_pkpy.py") == ALL_OK);
}

void test_python_922_py_compile(void** state)
{
    assert_true(test_python_script("922_py_compile.py") == ALL_OK);
}

void test_python_930_deterministic_float(void** state)
{
    assert_true(test_python_script("930_deterministic_float.py") == ALL_OK);
}

void test_python_950_bugs(void** state)
{
    assert_true(test_python_script("950_bugs.py") == ALL_OK);
}

void test_python_951_dis(void** state)
{
    assert_true(test_python_script("951_dis.py") == ALL_OK);
}

void test_python_960_pep695_py312(void** state)
{
    assert_true(test_python_script("960_pep695_py312.py") == ALL_OK);
}

void test_python_970_inspect(void** state)
{
    assert_true(test_python_script("970_inspect.py") == ALL_OK);
}

void test_python_980_thread(void** state)
{
    assert_true(test_python_script("980_thread.py") == ALL_OK);
}

void test_python_990_extras(void** state)
{
    assert_true(test_python_script("990_extras.py") == ALL_OK);
}

static status_t test_python_script(const char* script_name)
{
    status_t status;
    char script_path[1024] = {0};

    os_snprintf(script_path, sizeof(script_path), "tests/%s", script_name);

    status = run_script_ex(script_path, &test_core);

    return status;
}
