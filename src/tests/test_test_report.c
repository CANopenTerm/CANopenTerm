/** @file test_test_report.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cmocka.h"
#include "os.h"
#include "test_report.h"
#include "test_test_report.h"

void test_report_init(void** state)
{
	(void)state;

	/* test_init is idempotent — calling it twice must return ALL_OK. */
	assert_true(test_init() == ALL_OK);
	assert_true(test_init() == ALL_OK);

	test_clear_results();
}

void test_report_clear(void** state)
{
	test_result_t result = {0};

	(void)state;

	assert_true(test_init() == ALL_OK);

	result.has_passed  = true;
	result.package     = "Pkg";
	result.class_name  = "Cls";
	result.test_name   = "SomeTest";
	result.time        = 0.001f;

	test_add_result(&result);

	/* clear_results must not crash even when results exist. */
	test_clear_results();

	/* After clearing, generating a report must produce an empty-but-valid XML. */
	assert_true(test_init() == ALL_OK);
	assert_true(test_generate_report("tests/test_report_empty.xml") == ALL_OK);
}

void test_report_add_and_generate(void** state)
{
	test_result_t pass_result   = {0};
	test_result_t fail_result   = {0};
	FILE_t* f;

	(void)state;

	assert_true(test_init() == ALL_OK);

	/* Add a passing result. */
	pass_result.has_passed  = true;
	pass_result.package     = "CANopenTerm";
	pass_result.class_name  = "NMT";
	pass_result.test_name   = "test_nmt_send_command";
	pass_result.time        = 0.002f;
	test_add_result(&pass_result);

	/* Add a failing result with all optional fields populated. */
	fail_result.has_passed    = false;
	fail_result.package       = "CANopenTerm";
	fail_result.class_name    = "SDO";
	fail_result.test_name     = "test_sdo_read_timeout";
	fail_result.error_type    = "AssertionError";
	fail_result.error_message = "Expected ALL_OK but got SDO timeout";
	fail_result.call_stack    = "<!-- test_sdo_read_timeout:42 -->";
	fail_result.time          = 0.005f;
	test_add_result(&fail_result);

	/* Generate the XML report — must succeed and write a file. */
	assert_true(test_generate_report("tests/test_report_out.xml") == ALL_OK);

	/* Verify the output file was actually created and is non-empty. */
	f = os_fopen("tests/test_report_out.xml", "r");
	assert_non_null(f);
	os_fseek(f, 0, SEEK_END);
	assert_true(os_ftell(f) > 0);
	os_fclose(f);
}

void test_report_generate_null_filename(void** state)
{
	FILE_t* f;

	(void)state;

	assert_true(test_init() == ALL_OK);

	/* NULL filename must fall back to "test_report.xml" and still succeed. */
	assert_true(test_generate_report(NULL) == ALL_OK);

	f = os_fopen("test_report.xml", "r");
	assert_non_null(f);
	os_fclose(f);
}
