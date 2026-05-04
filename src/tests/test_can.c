/** @file test_can.c
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

#include "can.h"
#include "cmocka.h"
#include "core.h"
#include "os.h"
#include "test_can.h"

void test_can_limit_node_id(void** state)
{
	uint8 node_id;

	(void)state;

	node_id = 0x01;
	limit_node_id(&node_id);
	assert_int_equal(node_id, 0x01);

	node_id = 0x7f;
	limit_node_id(&node_id);
	assert_int_equal(node_id, 0x7f);

	node_id = 0x80;
	limit_node_id(&node_id);
	assert_int_equal(node_id, 0x7f);

	node_id = 0xff;
	limit_node_id(&node_id);
	assert_int_equal(node_id, 0x7f);
}

void test_can_is_can_initialised(void** state)
{
	core_t dummy_core = {0};

	(void)state;

	assert_false(is_can_initialised(NULL));

	dummy_core.is_can_initialised = false;
	assert_false(is_can_initialised(&dummy_core));

	dummy_core.is_can_initialised = true;
	assert_true(is_can_initialised(&dummy_core));
}
