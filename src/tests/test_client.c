/** @file test_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "client.h"
#include "cmocka.h"
#include "command.h"
#include "core.h"
#include "os.h"
#include "test_client.h"

/* Port 1 (tcpmux) is practically never listened on. */
#define TEST_UNUSED_PORT 1

static void run_command(const char* command, bool is_server_mode)
{
    core_t test_core = {0};
    char input[COMMAND_BUFFER_SIZE];
    core_t* previous_core = core;

    test_core.is_running = true;
    test_core.is_server_mode = is_server_mode;
    os_strlcpy(input, command, sizeof(input));

    core = &test_core;
    parse_command(input, &test_core, SILENT);
    core = previous_core;
}

void test_client_connect_refused(void** state)
{
    (void)state;

    assert_int_not_equal(ALL_OK, client_connect("127.0.0.1", TEST_UNUSED_PORT));
}

void test_client_connect_unresolvable_host(void** state)
{
    (void)state;

    assert_int_not_equal(ALL_OK, client_connect("host.invalid", TEST_UNUSED_PORT));
}

void test_parse_command_connect_invalid_port(void** state)
{
    (void)state;

    /* Rejected before any connection attempt is made. */
    run_command("o 127.0.0.1 0", false);
    run_command("o 127.0.0.1 70000", false);

    assert_true(true);
}

void test_parse_command_connect_server_mode(void** state)
{
    (void)state;

    /* Must be refused in server mode and must return immediately. */
    run_command("o 127.0.0.1 1", true);

    assert_true(true);
}

void test_parse_command_connect_refused(void** state)
{
    (void)state;

    run_command("o 127.0.0.1 1", false);

    assert_true(true);
}
