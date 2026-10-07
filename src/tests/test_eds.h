/** @file test_eds.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef TEST_EDS_H
#define TEST_EDS_H

/* EDS file operations and validation tests */
void test_list_eds(void** state);
void test_run_conformance_test_invalid_path(void** state);
void test_run_conformance_test_missing_package(void** state);
void test_validate_eds_invalid_file_no(void** state);

/* NOTE: EDS file handling involves file system I/O and external tool integration */
/* Complete validation requires end-to-end testing with actual EDS files and conformance tools */

#endif /* TEST_EDS_H */
