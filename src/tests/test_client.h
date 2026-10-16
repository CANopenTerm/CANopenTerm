/** @file test_client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

void test_client_connect_refused(void** state);
void test_client_connect_unresolvable_host(void** state);
void test_parse_command_connect_invalid_port(void** state);
void test_parse_command_connect_server_mode(void** state);
void test_parse_command_connect_refused(void** state);

#endif /* TEST_CLIENT_H */
