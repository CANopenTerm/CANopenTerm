/** @file test_dbc.c
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
#define os_mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define os_mkdir(dir, mode) mkdir(dir, mode)
#endif

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cmocka.h"
#include "dbc.h"
#include "os.h"
#include "test_dbc.h"

/* Minimal DBC with one message (ID 0x0CF00400 = 217056256, i.e. J1939-style)
 * and two signals so we can exercise decode and find. */
static const char* minimal_dbc =
	"VERSION \"\"\n"
	"\n"
	"NS_ :\n"
	"\n"
	"BS_:\n"
	"\n"
	"BU_:\n"
	"\n"
	"BO_ 100 TestMessage: 8 Vector__XXX\n"
	" SG_ EngineSpeed : 24|16@1+ (0.125,0) [0|8031.875] \"rpm\" Vector__XXX\n"
	" SG_ EngineLoad  :  8|8@1+ (1,0) [0|250] \"%\" Vector__XXX\n"
	"\n";

static const char* dbc_temp_path = "tests/temp_test.dbc";

/* ------------------------------------------------------------------ */
/* Guard-path tests: no DBC loaded                                     */
/* ------------------------------------------------------------------ */

void test_dbc_unloaded_guards(void** state)
{
	(void)state;

	/* dbc_decode with no DBC loaded must return empty string, not crash. */
	assert_string_equal(dbc_decode(100, 0, NULL), "");

	/* dbc_find_id_by_name with NULL args must return OS_INVALID_ARGUMENT. */
	assert_true(dbc_find_id_by_name(NULL, "name") == OS_INVALID_ARGUMENT);
	assert_true(dbc_find_id_by_name((uint32*)1, NULL) == OS_INVALID_ARGUMENT);

	/* dbc_unload on an already-unloaded DBC must not crash. */
	dbc_unload();

	/* dbc_print on an unloaded DBC must not crash. */
	dbc_print();
}

void test_dbc_load_invalid_path(void** state)
{
	(void)state;

	assert_true(dbc_load("nonexistent_file.dbc") == OS_FILE_NOT_FOUND);
}

/* ------------------------------------------------------------------ */
/* Full lifecycle: load → decode → find → print → unload              */
/* ------------------------------------------------------------------ */

void test_dbc_lifecycle(void** state)
{
	FILE_t* f;
	status_t status;

	(void)state;

	/* Write the minimal DBC to the temp file. */
	f = os_fopen(dbc_temp_path, "w");
	if (NULL == f)
	{
		/* Create the directory first. */
		char path_copy[256];
		os_strlcpy(path_copy, dbc_temp_path, sizeof(path_copy));
		*os_strrchr(path_copy, '/') = '\0';
#ifdef _WIN32
		_mkdir(path_copy);
#else
		os_mkdir(path_copy, 0777);
#endif
		f = os_fopen(dbc_temp_path, "w");
	}
	assert_non_null(f);
	os_fprintf(f, "%s", minimal_dbc);
	os_fclose(f);

	/* Load. */
	status = dbc_load((char*)dbc_temp_path);
	assert_true(status == ALL_OK);

	/* dbc_print must not crash. */
	dbc_print();

	/* Unload and clean up. */
	dbc_unload();
}

void test_dbc_decode_no_match(void** state)
{
	FILE_t* f;
	status_t status;
	const char* result;

	(void)state;

	f = os_fopen(dbc_temp_path, "w");
	assert_non_null(f);
	os_fprintf(f, "%s", minimal_dbc);
	os_fclose(f);

	status = dbc_load((char*)dbc_temp_path);
	assert_true(status == ALL_OK);

	/* ID 0x999 is not in the DBC — must return empty string. */
	result = dbc_decode(0x999, 0xDEADBEEFCAFEBABEULL, NULL);
	assert_string_equal(result, "");

	/* ID 100 is present — decode must return a non-empty string. */
	result = dbc_decode(100, 0x0000FF00000000FFULL, NULL);
	assert_non_null(result);
	assert_true(os_strlen(result) > 0);

	dbc_unload();
}

void test_dbc_find_id_invalid_args(void** state)
{
	FILE_t* f;
	status_t status;
	uint32 found_id = 0;

	(void)state;

	f = os_fopen(dbc_temp_path, "w");
	assert_non_null(f);
	os_fprintf(f, "%s", minimal_dbc);
	os_fclose(f);

	status = dbc_load((char*)dbc_temp_path);
	assert_true(status == ALL_OK);

	/* Known message name (case-insensitive substring match). */
	assert_true(dbc_find_id_by_name(&found_id, "TestMessage") == ALL_OK);
	assert_int_equal(found_id, 100);

	/* Unknown name must return ITEM_NOT_FOUND. */
	assert_true(dbc_find_id_by_name(&found_id, "DoesNotExist") == ITEM_NOT_FOUND);

	dbc_unload();
}
