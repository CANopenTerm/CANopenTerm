/** @file test_pdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_PDO_H
#define TEST_PDO_H

/* PDO validation tests */
void test_pdo_is_id_valid(void** state);
void test_pdo_print_help(void** state);

/* PDO add/delete operations tests */
/* NOTE: pdo_add() and pdo_del() require OS timer functionality and are integration tests */
/* NOTE: pdo_print_result() requires display output and is tested through integration tests */

#endif /* TEST_PDO_H */
