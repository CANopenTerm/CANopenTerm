/** @file test_table.c
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
#include "table.h"
#include "test_table.h"

void test_table_init(void** state)
{
	table_t t = {DEFAULT_COLOR, DEFAULT_COLOR, 10, 10, 10};

	(void)state;

	assert_true(table_init(&t, 1024) == ALL_OK);
	table_flush(&t);
}

void test_table_lifecycle(void** state)
{
	table_t t = {DARK_CYAN, DARK_WHITE, 16, 8, 8};

	(void)state;

	assert_true(table_init(&t, 512) == ALL_OK);

	table_print_header(&t);
	table_print_row("Index", "Sub", "Name", &t);
	table_print_divider(&t);
	table_print_row("0x1000", "0x00", "Device type", &t);
	table_print_row("0x1001", "0x00", "Error register", &t);
	table_print_footer(&t);
	table_flush(&t);
}
