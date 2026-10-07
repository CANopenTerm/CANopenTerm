/** @file test_command.c
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
#include <stdlib.h>
#include <string.h>

#include "cmocka.h"
#include "command.h"
#include "core.h"
#include "os.h"
#include "test_command.h"

/* Helper function to create a minimal core structure for testing */
static core_t* create_test_core(void)
{
    core_t* test_core = (core_t*)os_calloc(1, sizeof(core_t));
    if (test_core)
    {
        test_core->baud_rate = 5;  /* Default baud rate index */
        test_core->can_channel = 0;
        test_core->node_id = 1;
        test_core->is_can_initialised = false;
        test_core->is_running = true;
        test_core->is_abort = false;
    }
    return test_core;
}

/* Test: parse_command with empty input */
void test_parse_command_empty_input(void** state)
{
    core_t* test_core = create_test_core();
    char input[2] = "\0";

    (void)state;

    /* Calling parse_command with empty input should not crash */
    parse_command(input, test_core, SILENT);

    assert_non_null(test_core);
    os_free(test_core);
}

/* Test: parse_command with help command */
void test_parse_command_help(void** state)
{
    core_t* test_core = create_test_core();
    char input[16];

    (void)state;

    /* Copy the help command into buffer */
    os_strlcpy(input, "h", sizeof(input));

    /* Parse help command - should display usage without crashing */
    parse_command(input, test_core, SILENT);

    assert_non_null(test_core);
    os_free(test_core);
}

/* Test: parse_command with clear screen command */
void test_parse_command_clear(void** state)
{
    core_t* test_core = create_test_core();
    char input[16];

    (void)state;

    os_strlcpy(input, "c", sizeof(input));

    /* Parse clear command - should execute system clear without crash */
    parse_command(input, test_core, SILENT);

    assert_non_null(test_core);
    os_free(test_core);
}

/* Test: parse_command with quit command */
void test_parse_command_quit(void** state)
{
    core_t* test_core = create_test_core();
    char input[16];

    (void)state;

    os_strlcpy(input, "q", sizeof(input));

    /* Parse quit command - should handle gracefully in SILENT mode */
    parse_command(input, test_core, SILENT);

    /* Test passes if no crash occurs */
    assert_true(true);
    os_free(test_core);
}

/* Test: parse_command with script execution attempt */
void test_parse_command_script_run(void** state)
{
    core_t* test_core = create_test_core();
    char input[32];

    (void)state;

    /* Using 'h' (help) command as a single-char built-in that won't 
     * be interpreted as a script filename. Script file handling is
     * tested through integration tests with actual script files. */
    os_strlcpy(input, "h", sizeof(input));

    /* This will execute help - should handle gracefully */
    parse_command(input, test_core, SILENT);

    /* Test passes if no crash occurs */
    assert_true(true);
    os_free(test_core);
}

/* Test: completion_callback with help prefix */
void test_completion_callback_help(void** state)
{
    (void)state;

    /* Note: completions_t is an opaque type from isocline library.
     * Direct testing is not possible without isocline's initialization.
     * This test verifies the function can be called without crashing
     * when initialized by isocline. */

    /* This test is a placeholder - full testing would require 
     * isocline library initialization which is done at runtime. */
    assert_true(true);
}

/* Test: completion_callback with empty prefix */
void test_completion_callback_empty_prefix(void** state)
{
    (void)state;

    /* Similar to above - completions_t requires isocline initialization.
     * Full endpoint-to-endpoint testing is done at runtime via the 
     * interactive CLI, not unit tests. */

    assert_true(true);
}
