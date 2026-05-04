/** @file test_pdo.c
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
#include "pdo.h"
#include "test_pdo.h"

void test_pdo_is_id_valid(void** state)
{
	(void)state;

	/* Valid Node-ID range: 0x001 - 0x07f */
	assert_true(pdo_is_id_valid(0x001));
	assert_true(pdo_is_id_valid(0x040));
	assert_true(pdo_is_id_valid(0x07f));

	/* Invalid: zero */
	assert_false(pdo_is_id_valid(0x000));

	/* Invalid: base IDs excluded (0x180, 0x280, 0x380, 0x480) */
	assert_false(pdo_is_id_valid(0x180));
	assert_false(pdo_is_id_valid(0x280));
	assert_false(pdo_is_id_valid(0x380));
	assert_false(pdo_is_id_valid(0x480));

	/* Valid TPDO1 range: 0x181 - 0x1ff */
	assert_true(pdo_is_id_valid(0x181));
	assert_true(pdo_is_id_valid(0x1ff));

	/* Valid TPDO2 range: 0x281 - 0x2ff */
	assert_true(pdo_is_id_valid(0x281));
	assert_true(pdo_is_id_valid(0x2ff));

	/* Valid TPDO3 range: 0x381 - 0x3ff */
	assert_true(pdo_is_id_valid(0x381));
	assert_true(pdo_is_id_valid(0x3ff));

	/* Valid TPDO4 range: 0x481 - 0x4ff */
	assert_true(pdo_is_id_valid(0x481));
	assert_true(pdo_is_id_valid(0x4ff));

	/* Invalid: above TPDO4 */
	assert_false(pdo_is_id_valid(0x500));
	assert_false(pdo_is_id_valid(0x7ff));

	/* Invalid: between Node-ID and TPDO1 */
	assert_false(pdo_is_id_valid(0x080));
	assert_false(pdo_is_id_valid(0x17f));
}
