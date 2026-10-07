/** @file test_sdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_SDO_H
#define TEST_SDO_H

/* SDO abort code lookup tests */
void test_sdo_lookup_abort_code(void** state);

/* SDO read/write operation tests */
/* NOTE: sdo_read(), sdo_write(), sdo_write_block(), and sdo_write_segmented() */
/* require actual CAN hardware communication and are tested through integration tests only. */
/* Unit testing these would require mocking the entire CAN message exchange protocol. */
void test_sdo_read_null_check(void** state);
void test_sdo_write_boundary_values(void** state);

#endif /* TEST_SDO_H */
