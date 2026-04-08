/** @file test_scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_SCRIPTS_H
#define TEST_SCRIPTS_H

void test_has_valid_extension(void** state);
void test_lua(void** state);
void test_python_010_int(void** state);
void test_python_020_float(void** state);
void test_python_030_bool(void** state);
void test_python_040_line_continue(void** state);
void test_python_041_str(void** state);
void test_python_042_str_mod(void** state);
void test_python_050_list(void** state);
void test_python_060_tuple(void** state);
void test_python_070_listcomp(void** state);
void test_python_080_dict(void** state);
void test_python_081_dictcomp(void** state);
void test_python_150_assign(void** state);
void test_python_151_cmp(void** state);
void test_python_152_controlflow(void** state);
void test_python_160_functions(void** state);
void test_python_161_typehints(void** state);
void test_python_240_inline_blocks(void** state);
void test_python_250_rfstring(void** state);
void test_python_260_multiline(void** state);
void test_python_280_exception(void** state);
void test_python_290_iter(void** state);
void test_python_300_import(void** state);
void test_python_301_import1(void** state);
void test_python_310_modulereload(void** state);
void test_python_400_class(void** state);
void test_python_410_class_ex(void** state);
void test_python_420_decorator(void** state);
void test_python_430_closure(void** state);
void test_python_440_star(void** state);
void test_python_460_bytes(void** state);
void test_python_470_set(void** state);
void test_python_480_setcomp(void** state);
void test_python_500_reflection(void** state);
void test_python_510_yield(void** state);
void test_python_520_context(void** state);
void test_python_660_eval(void** state);
void test_python_661_exec_bug(void** state);
void test_python_670_locals_vs_globals(void** state);
void test_python_700_base64(void** state);
void test_python_701_bisect(void** state);
void test_python_702_builtins(void** state);
void test_python_703_heapq(void** state);
void test_python_704_math(void** state);
void test_python_705_random(void** state);
void test_python_710_cmath(void** state);
void test_python_711_gc(void** state);
void test_python_720_collections(void** state);
void test_python_721_json(void** state);
void test_python_722_lz4(void** state);
void test_python_723_msgpack(void** state);
void test_python_730_functools(void** state);
void test_python_731_json_alt(void** state);
void test_python_732_json_indent(void** state);
void test_python_733_typing(void** state);
void test_python_740_operator(void** state);
void test_python_750_compile(void** state);
void test_python_760_dna(void** state);
void test_python_761_misc(void** state);
void test_python_762_prime(void** state);
void test_python_770_builtin_func_1(void** state);
void test_python_771_builtin_func_2(void** state);
void test_python_790_datetime(void** state);
void test_python_791_easing(void** state);
void test_python_792_file(void** state);
void test_python_793_stdc(void** state);
void test_python_800_color32(void** state);
void test_python_801_sys(void** state);
void test_python_802_traceback(void** state);
void test_python_803_vmath(void** state);
void test_python_810_dataclasses(void** state);
void test_python_820_enum(void** state);
void test_python_830_unicodedata(void** state);
void test_python_900_array2d(void** state);
void test_python_901_array2d_extra1(void** state);
void test_python_902_chunked_array2d(void** state);
void test_python_903_colorcvt(void** state);
void test_python_904_pickle(void** state);
void test_python_910_line_profiler(void** state);
void test_python_920_picoterm(void** state);
void test_python_921_pkpy(void** state);
void test_python_922_py_compile(void** state);
void test_python_930_deterministic_float(void** state);
void test_python_950_bugs(void** state);
void test_python_951_dis(void** state);
void test_python_960_pep695_py312(void** state);
void test_python_970_inspect(void** state);
void test_python_980_thread(void** state);
void test_python_990_extras(void** state);

#endif /* TEST_SCRIPTS_H */
