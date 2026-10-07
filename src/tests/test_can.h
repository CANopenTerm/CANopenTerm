/** @file test_can.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_CAN_H
#define TEST_CAN_H

/* CAN core functionality tests */
void test_can_limit_node_id(void** state);
void test_can_is_can_initialised(void** state);

/* CAN initialization and error handling tests */
/* NOTE: can_init() and can_deinit() require hardware initialization and are tested indirectly */
/* through integration tests. They cannot be unit tested in isolation without full CAN hardware. */
void test_can_get_error_message(void** state);
void test_can_error_message_invalid_code(void** state);

/* NOTE: can_print_error() requires display output mocking and is tested through integration tests */
/* NOTE: can_read() and can_write() require CAN hardware and are integration tests only */
/* NOTE: can_flush() is a simple hardware buffer flush without testable state changes */
/* NOTE: can_quit() modifies global state and is tested through core lifecycle tests */
/* NOTE: can_set_baud_rate() and can_set_channel() require hardware and integration testing */
/* NOTE: can_print_baud_rate_help() and can_print_channel_help() test output formatting (indirect tests exist) */

#endif /* TEST_CAN_H */
