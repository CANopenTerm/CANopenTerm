/** @file test_sdo_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier),  MIT
 *
 **/

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "cmocka.h"
#include "sdo.h"
#include "test_sdo.h"

void test_sdo_lookup_abort_code(void** state)
{
    (void)state;

    assert_string_equal(sdo_lookup_abort_code(ABORT_TOGGLE_BIT_NOT_ALTERED), "Toggle bit not altered");
    assert_string_equal(sdo_lookup_abort_code(ABORT_SDO_PROTOCOL_TIMED_OUT), "SDO protocol timed out");
    assert_string_equal(sdo_lookup_abort_code(ABORT_CMD_SPECIFIER_INVALID_UNKNOWN), "Client/server command specifier not valid or unknown");
    assert_string_equal(sdo_lookup_abort_code(ABORT_INVALID_BLOCK_SIZE), "Invalid block size");
    assert_string_equal(sdo_lookup_abort_code(ABORT_INVALID_SEQUENCE_NUMBER), "Invalid sequence number");
    assert_string_equal(sdo_lookup_abort_code(ABORT_CRC_ERROR), "CRC error");
    assert_string_equal(sdo_lookup_abort_code(ABORT_OUT_OF_MEMORY), "Out of memory");
    assert_string_equal(sdo_lookup_abort_code(ABORT_UNSUPPORTED_ACCESS), "Unsupported access to an object");
    assert_string_equal(sdo_lookup_abort_code(ABORT_ATTEMPT_TO_READ_WRITE_ONLY), "Attempt to read a write only object");
    assert_string_equal(sdo_lookup_abort_code(ABORT_ATTEMPT_TO_WRITE_READ_ONLY), "Attempt to write a read only object");
    assert_string_equal(sdo_lookup_abort_code(ABORT_OBJECT_DOES_NOT_EXIST), "Object does not exist in the object dictionary");
    assert_string_equal(sdo_lookup_abort_code(ABORT_OBJECT_CANNOT_BE_MAPPED), "Object cannot be mapped to the PDO");
    assert_string_equal(sdo_lookup_abort_code(ABORT_WOULD_EXCEED_PDO_LENGTH), "Number, length of the object would exceed PDO length");
    assert_string_equal(sdo_lookup_abort_code(ABORT_GENERAL_INCOMPATIBILITY_REASON), "General parameter incompatibility reason");
    assert_string_equal(sdo_lookup_abort_code(ABORT_GENERAL_INTERNAL_INCOMPATIBILITY), "General internal incompatibility in the device");
    assert_string_equal(sdo_lookup_abort_code(ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR), "Access failed due to an hardware error");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_TYPE_DOES_NOT_MATCH), "Data type does not match, length does not match");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_TYPE_LENGTH_TOO_HIGH), "Data type does not match, length too high");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_TYPE_LENGTH_TOO_LOW), "Data type does not match, length too low");
    assert_string_equal(sdo_lookup_abort_code(ABORT_SUB_INDEX_DOES_NOT_EXIST), "Sub-index does not exist");
    assert_string_equal(sdo_lookup_abort_code(ABORT_INVALID_VALUE_FOR_PARAMETER), "Invalid value for parameter");
    assert_string_equal(sdo_lookup_abort_code(ABORT_VALUE_FOR_PARAMETER_TOO_HIGH), "Value for parameter written too high");
    assert_string_equal(sdo_lookup_abort_code(ABORT_VALUE_FOR_PARAMETER_TOO_LOW), "Value for parameter written too low");
    assert_string_equal(sdo_lookup_abort_code(ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE), "Maximum value is less than minimum value");
    assert_string_equal(sdo_lookup_abort_code(ABORT_RESOURCE_NOT_AVAILABLE), "Resource not available: SDO connection");
    assert_string_equal(sdo_lookup_abort_code(ABORT_GENERAL_ERROR), "General error");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_CANNOT_BE_TRANSFERRED), "Data cannot be transferred");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL), "Data cannot be transferred or stored to the application because of local control");
    assert_string_equal(sdo_lookup_abort_code(ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE), "Data cannot be transferred because of the present device state");
    assert_string_equal(sdo_lookup_abort_code(ABORT_NO_OBJECT_DICTIONARY_PRESENT), "Object dictionary dynamic generation fails or no object dictionary present");
    assert_string_equal(sdo_lookup_abort_code(ABORT_NO_DATA_AVAILABLE), "No data available");
    assert_string_equal(sdo_lookup_abort_code(0x12345678), "Unknown abort code");
}
