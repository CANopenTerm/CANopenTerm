/** @file test_scripts.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_SCRIPTS_H
#define TEST_SCRIPTS_H

void test_has_valid_extension(void** state);
void test_lua(void** state);
void test_python_01_int(void** state);
void test_python_02_float(void** state);
void test_python_03_bool(void** state);
void test_python_04_line_continue(void** state);
void test_python_04_str(void** state);
void test_python_05_list(void** state);
void test_python_06_tuple(void** state);
void test_python_07_listcomp(void** state);
void test_python_08_dictcomp(void** state);
void test_python_08_dict(void** state);
void test_python_15_assign(void** state);
void test_python_15_cmp(void** state);
void test_python_15_controlflow(void** state);
void test_python_16_functions(void** state);
void test_python_16_typehints(void** state);
void test_python_24_inline_blocks(void** state);
void test_python_25_rfstring(void** state);
void test_python_26_multiline(void** state);
void test_python_28_exception(void** state);
void test_python_29_iter(void** state);
void test_python_30_import(void** state);
void test_python_40_class(void** state);
void test_python_41_class_ex(void** state);
void test_python_42_decorator(void** state);
void test_python_43_closure(void** state);
void test_python_44_star(void** state);
void test_python_46_bytes(void** state);
void test_python_47_set(void** state);
void test_python_48_setcomp(void** state);
void test_python_50_reflection(void** state);
void test_python_51_yield(void** state);
void test_python_52_context(void** state);
void test_python_66_eval(void** state);
void test_python_70_bisect(void** state);
void test_python_70_builtins(void** state);
void test_python_70_heapq(void** state);
void test_python_70_math(void** state);
void test_python_70_random(void** state);
void test_python_71_cmath(void** state);
void test_python_71_gc(void** state);
void test_python_72_collections(void** state);
void test_python_72_json(void** state);
void test_python_73_functools(void** state);
void test_python_73_json_alt(void** state);
void test_python_73_typing(void** state);
void test_python_74_operator(void** state);
void test_python_75_compile(void** state);
void test_python_76_dna(void** state);
void test_python_76_misc(void** state);
void test_python_76_prime(void** state);
void test_python_77_builtin_func(void** state);
void test_python_79_datetime(void** state);
void test_python_79_easing(void** state);
void test_python_79_file(void** state);
void test_python_80_linalg(void** state);
void test_python_80_sys(void** state);
void test_python_80_traceback(void** state);
void test_python_81_dataclasses(void** state);
void test_python_82_enum(void** state);
void test_python_90_array2d(void** state);
void test_python_90_pickle(void** state);
void test_python_91_line_profiler(void** state);
void test_python_91_pdb(void** state);
void test_python_95_bugs(void** state);
void test_python_95_dis(void** state);
void test_python_96_pep695_py312(void** state);
void test_python_99_extras(void** state);

#endif /* TEST_SCRIPTS_H */
