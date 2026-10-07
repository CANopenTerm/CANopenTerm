/** @file test_command.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_COMMAND_H
#define TEST_COMMAND_H

void test_parse_command_empty_input(void** state);
void test_parse_command_help(void** state);
void test_parse_command_clear(void** state);
void test_parse_command_quit(void** state);
void test_parse_command_script_run(void** state);
void test_completion_callback_help(void** state);
void test_completion_callback_empty_prefix(void** state);

#endif /* TEST_COMMAND_H */
