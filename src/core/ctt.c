/** @file ctt.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "ctt.h"
#include "can.h"
#include "codb.h"
#include "eds.h"
#include "sdo.h"
#include "test_report.h"

status_t cct_run_test(uint32 node_id, const char* report_file)
{
    status_t status = ALL_OK;
    codb_t* ds301_db = codb_get_ds301_profile();
    codb_t* extended_db = codb_get_profile();
    can_message_t sdo_response = {0};
    uint32 passed_count = 0;
    uint32 failed_count = 0;
    uint32 test_count = 0;
    cJSON* object = NULL;

    os_printf("================================================================================\n");
    os_printf("                   CANopen Conformance Test (CiA 301)\n");
    os_printf("================================================================================\n");
    os_printf("Node ID: 0x%02X\n", node_id);
    os_printf("--------------------------------------------------------------------------------\n");
    os_printf("\n");

    if (ds301_db == NULL)
    {
        os_log(LOG_ERROR, "DS301 profile not loaded.");
        return ITEM_NOT_FOUND;
    }

    test_init();

    os_printf("Testing DS301 Mandatory Objects:\n");
    os_printf("----------------\n");

    /* Iterate through all objects in the DS301 profile. */
    cJSON_ArrayForEach(object, ds301_db)
    {
        cJSON* json_index = cJSON_GetObjectItem(object, "index");
        cJSON* json_category = cJSON_GetObjectItem(object, "kind");
        cJSON* json_desc = cJSON_GetObjectItem(object, "desc");

        if (json_index != NULL && json_category != NULL)
        {
            uint16 index = (uint16)json_index->valueint;
            int category = json_category->valueint;
            const char* desc = (json_desc != NULL && json_desc->valuestring != NULL) ? json_desc->valuestring : "Unknown";

            /* Test mandatory objects (IS_MANDATORY = 1). */
            if (category == IS_MANDATORY)
            {
                uint8 sub_index;

                /* Get base object info to determine number of sub-indices. */
                object_info_t base_info = {0};
                codb_info_lookup(ds301_db, index, 0, &base_info);

                /* Determine number of sub-indices to test. */
                int num_sub_indices = base_info.entry_count;
                if (num_sub_indices <= 0)
                {
                    num_sub_indices = 1; /* At least test sub-index 0. */
                }

                /* Iterate through all sub-indices. */
                for (sub_index = 0; sub_index < num_sub_indices; sub_index++)
                {
                    bool is_compliant;
                    bool read_compliant;
                    bool read_should_succeed;
                    bool read_succeeded;
                    bool write_compliant;

                    char error_msg[256] = {0};
                    char test_name[96] = {0};

                    const char* read_failure_reason = NULL;
                    const char* write_failure_reason = NULL;

                    object_info_t obj_info = {0};
                    sdo_state_t sdo_state;
                    test_result_t result = {0};

                    test_count++;

                    /* Get access type information for this sub-index. */
                    codb_info_lookup(ds301_db, index, sub_index, &obj_info);

                    /* Send SDO read request to verify sub-index exists. */
                    sdo_state = sdo_read(&sdo_response, SILENT, node_id, index, sub_index, desc);

                    /* Format test name and description with sub-index. */
                    if (sub_index == 0)
                    {
                        os_snprintf(test_name, sizeof(test_name), "0x%04X", index);
                    }
                    else
                    {
                        os_snprintf(test_name, sizeof(test_name), "0x%04X:%02X", index, sub_index);
                    }

                    result.test_name = test_name;
                    result.class_name = "DS301_Mandatory";
                    result.package = "CANopenTerm.Conformance";
                    result.time = 0.0f;
                    result.call_stack = NULL;

                    /* Check read compliance. */
                    read_succeeded = (sdo_state == IS_READ_EXPEDITED || sdo_state == IS_READ_SEGMENTED || sdo_state == IS_READ_BLOCK);
                    read_should_succeed = (obj_info.access_type == CONST_T || obj_info.access_type == RO || obj_info.access_type == RW);
                    read_compliant = (read_succeeded == read_should_succeed);

                    if (! read_compliant)
                    {
                        if (sdo_state == ABORT_TRANSFER && ! read_should_succeed)
                        {
                            read_compliant = true; /* Abort is expected for WO. */
                        }
                        else if (sdo_state == ABORT_TRANSFER)
                        {
                            uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                            abort_code = os_swap_be_32(abort_code);
                            read_failure_reason = sdo_lookup_abort_code(abort_code);
                        }
                        else
                        {
                            read_failure_reason = "Read operation failed";
                        }
                    }

                    /* Check write compliance (only if read test passed, to avoid cascading failures). */
                    write_compliant = true;

                    if (read_succeeded)
                    {
                        sdo_state_t write_state;
                        uint32 test_value = 0;
                        uint32 write_size = codb_get_data_type_size(obj_info.data_type);
                        if (write_size == 0)
                        {
                            write_size = sizeof(test_value); /* Default to 4 bytes for variable-length types. */
                        }
                        write_state = sdo_write(&sdo_response, SILENT, node_id, index, sub_index, write_size, &test_value, desc);

                        bool write_should_succeed = (obj_info.access_type == RW || obj_info.access_type == WO);
                        bool write_succeeded = (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK);
                        write_compliant = (write_succeeded == write_should_succeed);

                        if (! write_compliant)
                        {
                            if (write_state == ABORT_TRANSFER)
                            {
                                uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                                abort_code = os_swap_be_32(abort_code);
                                write_failure_reason = sdo_lookup_abort_code(abort_code);
                            }
                            else if (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK)
                            {
                                write_failure_reason = "Write succeeded but should have been rejected";
                            }
                            else
                            {
                                write_failure_reason = "Write operation not supported";
                            }
                        }
                    }

                    /* Determine overall result. */
                    is_compliant = read_compliant && write_compliant;
                    result.has_passed = is_compliant;

                    if (is_compliant)
                    {
                        result.error_type = NULL;
                        result.error_message = NULL;
                        passed_count++;
                    }
                    else
                    {
                        result.error_type = "AccessTypeViolation";
                        if (! read_compliant && ! write_compliant)
                        {
                            os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed (%s), Write failed (%s)",
                                        access_type_lookup[obj_info.access_type], read_failure_reason, write_failure_reason);
                        }
                        else if (! read_compliant)
                        {
                            os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed (%s)",
                                        access_type_lookup[obj_info.access_type], read_failure_reason);
                        }
                        else
                        {
                            os_snprintf(error_msg, sizeof(error_msg), "%s object: Write failed (%s)",
                                        access_type_lookup[obj_info.access_type], write_failure_reason);
                        }
                        result.error_message = error_msg;
                        failed_count++;
                    }

                    /* Output test result. */
                    if (! result.has_passed)
                    {
                        const char* indent = (sub_index == 0) ? "" : "    ";
                        os_printf("  %s[!!] 0x%04X:%02X - %s: %s\n", indent, index, sub_index, obj_info.sub_index_name, result.error_message);
                    }
                    else
                    {
                        const char* indent = (sub_index == 0) ? "" : "    ";
                        os_printf("  %s[OK] 0x%04X:%02X - %s\n", indent, index, sub_index, obj_info.sub_index_name);
                    }
                    test_add_result(&result);
                }
            }
        }
    }

    os_printf("\n");
    os_printf("Testing DS301 Optional Objects:\n");
    os_printf("----------------\n");

    /* Iterate through optional objects in the DS301 profile. */
    cJSON_ArrayForEach(object, ds301_db)
    {
        cJSON* json_index = cJSON_GetObjectItem(object, "index");
        cJSON* json_category = cJSON_GetObjectItem(object, "kind");
        cJSON* json_desc = cJSON_GetObjectItem(object, "desc");

        if (json_index != NULL && json_category != NULL)
        {
            uint16 index = (uint16)json_index->valueint;
            int category = json_category->valueint;
            const char* desc = (json_desc != NULL && json_desc->valuestring != NULL) ? json_desc->valuestring : "Unknown";

            /* Test optional objects (IS_OPTIONAL = 0). */
            if (category == IS_OPTIONAL)
            {
                uint8 sub_index;
                int num_sub_indices;

                /* Get base object info to determine number of sub-indices. */
                object_info_t base_info = {0};
                codb_info_lookup(ds301_db, index, 0, &base_info);

                /* Determine number of sub-indices to test. */
                num_sub_indices = base_info.entry_count;
                if (num_sub_indices <= 0)
                {
                    num_sub_indices = 1; /* At least test sub-index 0. */
                }

                /* Iterate through all sub-indices. */
                for (sub_index = 0; sub_index < num_sub_indices; sub_index++)
                {
                    bool read_succeeded;
                    char error_msg[256] = {0};
                    char test_name[128] = {0};
                    object_info_t obj_info = {0};
                    sdo_state_t sdo_state;
                    test_result_t result = {0};

                    test_count++;

                    /* Get access type information for this sub-index. */
                    codb_info_lookup(ds301_db, index, sub_index, &obj_info);

                    /* Send SDO read request to verify sub-index exists. */
                    sdo_state = sdo_read(&sdo_response, SILENT, node_id, index, sub_index, desc);

                    /* Format test name with sub-index. */
                    if (sub_index == 0)
                    {
                        os_snprintf(test_name, sizeof(test_name), "0x%04X (Optional)", index);
                    }
                    else
                    {
                        os_snprintf(test_name, sizeof(test_name), "0x%04X:%02X (Optional)", index, sub_index);
                    }

                    result.test_name = test_name;
                    result.class_name = "DS301_Optional";
                    result.package = "CANopenTerm.Conformance";
                    result.time = 0.0f;
                    result.call_stack = NULL;

                    /* Check if sub-index exists. */
                    read_succeeded = (sdo_state == IS_READ_EXPEDITED || sdo_state == IS_READ_SEGMENTED || sdo_state == IS_READ_BLOCK);

                    if (! read_succeeded && sdo_state != ABORT_TRANSFER)
                    {
                        /* Optional sub-index doesn't exist; this is acceptable. */
                        result.has_passed = true;
                        result.error_type = "OptionalNotAvailable";
                        result.error_message = "Optional object not available";
                        passed_count++;
                        test_add_result(&result);
                    }
                    else if (! read_succeeded && sdo_state == ABORT_TRANSFER)
                    {
                        /* Optional sub-index doesn't exist (abort); this is acceptable. */
                        uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                        const char* abort_desc;

                        abort_code = os_swap_be_32(abort_code);
                        abort_desc = sdo_lookup_abort_code(abort_code);

                        result.has_passed = true;
                        result.error_type = "OptionalNotImplemented";
                        result.error_message = abort_desc;
                        passed_count++;
                        test_add_result(&result);
                    }
                    else
                    {
                        /* Sub-index exists; test both read and write compliance. */
                        bool is_compliant;
                        bool read_should_succeed = (obj_info.access_type == CONST_T || obj_info.access_type == RO || obj_info.access_type == RW);
                        bool read_compliant = (read_succeeded == read_should_succeed);
                        bool write_compliant;
                        bool write_should_succeed;
                        bool write_succeeded;

                        const char* indent;
                        const char* write_failure_reason = NULL;

                        sdo_state_t write_state;

                        uint32 test_value;
                        uint32 write_size;

                        if (obj_info.access_type == WO && sdo_state == ABORT_TRANSFER)
                        {
                            read_compliant = true; /* WO correctly rejected read. */
                        }

                        /* Check write compliance (only if sub-index exists). */
                        write_compliant = true;

                        test_value = 0;
                        write_size = codb_get_data_type_size(obj_info.data_type);
                        if (write_size == 0)
                        {
                            write_size = sizeof(test_value); /* Default to 4 bytes for variable-length types. */
                        }
                        write_state = sdo_write(&sdo_response, SILENT, node_id, index, sub_index, write_size, &test_value, desc);

                        write_should_succeed = (obj_info.access_type == RW || obj_info.access_type == WO);
                        write_succeeded = (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK);
                        write_compliant = (write_succeeded == write_should_succeed);

                        if (! write_compliant)
                        {
                            if (write_state == ABORT_TRANSFER)
                            {
                                uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                                abort_code = os_swap_be_32(abort_code);
                                write_failure_reason = sdo_lookup_abort_code(abort_code);
                            }
                            else if (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK)
                            {
                                write_failure_reason = "Write succeeded but should have been rejected";
                            }
                            else
                            {
                                write_failure_reason = "Write operation not supported";
                            }
                        }

                        /* Determine overall result. */
                        is_compliant = read_compliant && write_compliant;
                        result.has_passed = is_compliant;

                        if (is_compliant)
                        {
                            result.error_type = NULL;
                            result.error_message = NULL;
                            passed_count++;
                        }
                        else
                        {
                            result.error_type = "AccessTypeViolation";
                            if (! read_compliant && ! write_compliant)
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed, Write failed (%s)",
                                            access_type_lookup[obj_info.access_type], write_failure_reason);
                            }
                            else if (! read_compliant)
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed",
                                            access_type_lookup[obj_info.access_type]);
                            }
                            else
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Write failed (%s)",
                                            access_type_lookup[obj_info.access_type], write_failure_reason);
                            }
                            result.error_message = error_msg;
                            failed_count++;
                        }

                        /* Indent for sub-indices. */
                        indent = (sub_index == 0) ? "" : "    ";
                        /* Only output if test failed. */
                        if (! result.has_passed)
                        {
                            os_printf("  %s[!!] 0x%04X:%02X - %s: %s\n", indent, index, sub_index, obj_info.sub_index_name, result.error_message);
                        }
                        test_add_result(&result);
                    }
                }
            }
        }
    }

    /* Test extended database if it's loaded and different from DS301. */
    if (extended_db != NULL && extended_db != ds301_db)
    {
        os_printf("\n");
        os_printf("Testing Extended Database Mandatory Objects:\n");
        os_printf("----------------\n");

        cJSON_ArrayForEach(object, extended_db)
        {
            cJSON* json_index = cJSON_GetObjectItem(object, "index");
            cJSON* json_category = cJSON_GetObjectItem(object, "kind");
            cJSON* json_desc = cJSON_GetObjectItem(object, "desc");

            if (json_index != NULL && json_category != NULL)
            {
                uint16 index = (uint16)json_index->valueint;
                int category = json_category->valueint;
                const char* desc = (json_desc != NULL && json_desc->valuestring != NULL) ? json_desc->valuestring : "Unknown";

                /* Test mandatory objects (IS_MANDATORY = 1) from extended database. */
                if (category == IS_MANDATORY)
                {
                    uint8 sub_index;
                    int num_sub_indices;

                    /* Get base object info to determine number of sub-indices. */
                    object_info_t base_info = {0};
                    codb_info_lookup(extended_db, index, 0, &base_info);

                    /* Determine number of sub-indices to test. */
                    num_sub_indices = base_info.entry_count;
                    if (num_sub_indices <= 0)
                    {
                        num_sub_indices = 1; /* At least test sub-index 0. */
                    }

                    /* Iterate through all sub-indices. */
                    for (sub_index = 0; sub_index < num_sub_indices; sub_index++)
                    {
                        bool is_compliant;
                        bool read_compliant;
                        bool read_should_succeed;
                        bool read_succeeded;
                        bool write_compliant;

                        char error_msg[256] = {0};
                        char test_name[96] = {0};

                        const char* read_failure_reason = NULL;
                        const char* write_failure_reason = NULL;

                        object_info_t obj_info = {0};
                        sdo_state_t sdo_state;
                        test_result_t result = {0};

                        test_count++;

                        /* Get access type information for this sub-index. */
                        codb_info_lookup(extended_db, index, sub_index, &obj_info);

                        /* Send SDO read request to verify sub-index exists. */
                        sdo_state = sdo_read(&sdo_response, SILENT, node_id, index, sub_index, desc);

                        /* Format test name and description with sub-index. */
                        if (sub_index == 0)
                        {
                            os_snprintf(test_name, sizeof(test_name), "0x%04X", index);
                        }
                        else
                        {
                            os_snprintf(test_name, sizeof(test_name), "0x%04X:%02X", index, sub_index);
                        }

                        result.test_name = test_name;
                        result.class_name = "ExtendedDB_Mandatory";
                        result.package = "CANopenTerm.Conformance";
                        result.time = 0.0f;
                        result.call_stack = NULL;

                        /* Check read compliance. */
                        read_succeeded = (sdo_state == IS_READ_EXPEDITED || sdo_state == IS_READ_SEGMENTED || sdo_state == IS_READ_BLOCK);
                        read_should_succeed = (obj_info.access_type == CONST_T || obj_info.access_type == RO || obj_info.access_type == RW);
                        read_compliant = (read_succeeded == read_should_succeed);

                        if (! read_compliant)
                        {
                            if (sdo_state == ABORT_TRANSFER && ! read_should_succeed)
                            {
                                read_compliant = true; /* Abort is expected for WO. */
                            }
                            else if (sdo_state == ABORT_TRANSFER)
                            {
                                uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) |
                                                    (sdo_response.data[6] << 8) | sdo_response.data[7];
                                abort_code = os_swap_be_32(abort_code);
                                read_failure_reason = sdo_lookup_abort_code(abort_code);
                            }
                            else
                            {
                                read_failure_reason = "Read operation failed";
                            }
                        }

                        /* Check write compliance (only if read test passed, to avoid cascading failures). */
                        write_compliant = true;

                        if (read_succeeded)
                        {
                            sdo_state_t write_state;
                            uint32 test_value = 0;
                            uint32 write_size = codb_get_data_type_size(obj_info.data_type);

                            bool write_should_succeed;
                            bool write_succeeded;

                            if (write_size == 0)
                            {
                                write_size = sizeof(test_value); /* Default to 4 bytes for variable-length types. */
                            }
                            write_state = sdo_write(&sdo_response, SILENT, node_id, index, sub_index, write_size, &test_value, desc);

                            write_should_succeed = (obj_info.access_type == RW || obj_info.access_type == WO);
                            write_succeeded = (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK);
                            write_compliant = (write_succeeded == write_should_succeed);

                            if (! write_compliant)
                            {
                                if (write_state == ABORT_TRANSFER)
                                {
                                    uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                                    abort_code = os_swap_be_32(abort_code);
                                    write_failure_reason = sdo_lookup_abort_code(abort_code);
                                }
                                else if (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK)
                                {
                                    write_failure_reason = "Write succeeded but should have been rejected";
                                }
                                else
                                {
                                    write_failure_reason = "Write operation not supported";
                                }
                            }
                        }

                        /* Determine overall result. */
                        is_compliant = read_compliant && write_compliant;
                        result.has_passed = is_compliant;

                        if (is_compliant)
                        {
                            result.error_type = NULL;
                            result.error_message = NULL;
                            passed_count++;
                        }
                        else
                        {
                            result.error_type = "AccessTypeViolation";
                            if (! read_compliant && ! write_compliant)
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed (%s), Write failed (%s)",
                                            access_type_lookup[obj_info.access_type], read_failure_reason, write_failure_reason);
                            }
                            else if (! read_compliant)
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed (%s)",
                                            access_type_lookup[obj_info.access_type], read_failure_reason);
                            }
                            else
                            {
                                os_snprintf(error_msg, sizeof(error_msg), "%s object: Write failed (%s)",
                                            access_type_lookup[obj_info.access_type], write_failure_reason);
                            }
                            result.error_message = error_msg;
                            failed_count++;
                        }

                        /* Output test result */
                        if (! result.has_passed)
                        {
                            const char* indent = (sub_index == 0) ? "" : "    ";
                            os_printf("  %s[!!] 0x%04X:%02X - %s: %s\n", indent, index, sub_index, obj_info.sub_index_name, result.error_message);
                        }
                        else
                        {
                            const char* indent = (sub_index == 0) ? "" : "    ";
                            os_printf("  %s[OK] 0x%04X:%02X - %s\n", indent, index, sub_index, obj_info.sub_index_name);
                        }
                        test_add_result(&result);
                    }
                }
            }
        }

        os_printf("Testing Extended Database Optional Objects:\n");
        os_printf("----------------\n");

        cJSON_ArrayForEach(object, extended_db)
        {
            cJSON* json_index = cJSON_GetObjectItem(object, "index");
            cJSON* json_category = cJSON_GetObjectItem(object, "kind");
            cJSON* json_desc = cJSON_GetObjectItem(object, "desc");

            if (json_index != NULL && json_category != NULL)
            {
                uint16 index = (uint16)json_index->valueint;
                int category = json_category->valueint;
                const char* desc = (json_desc != NULL && json_desc->valuestring != NULL) ? json_desc->valuestring : "Unknown";

                /* Test optional objects (IS_OPTIONAL = 0) from extended database. */
                if (category == IS_OPTIONAL)
                {
                    uint8 sub_index;

                    /* Get base object info to determine number of sub-indices. */
                    object_info_t base_info = {0};
                    int num_sub_indices;
                    codb_info_lookup(extended_db, index, 0, &base_info);

                    /* Determine number of sub-indices to test. */
                    num_sub_indices = base_info.entry_count;
                    if (num_sub_indices <= 0)
                    {
                        num_sub_indices = 1; /* At least test sub-index 0. */
                    }

                    /* Iterate through all sub-indices. */
                    for (sub_index = 0; sub_index < num_sub_indices; sub_index++)
                    {
                        sdo_state_t sdo_state;
                        test_result_t result = {0};
                        char test_name[128] = {0};
                        char error_msg[256] = {0};
                        object_info_t obj_info = {0};

                        bool read_succeeded;

                        test_count++;

                        /* Get access type information for this sub-index. */
                        codb_info_lookup(extended_db, index, sub_index, &obj_info);

                        /* Send SDO read request to verify sub-index exists. */
                        sdo_state = sdo_read(&sdo_response, SILENT, node_id, index, sub_index, desc);

                        /* Format test name with sub-index. */
                        if (sub_index == 0)
                        {
                            os_snprintf(test_name, sizeof(test_name), "0x%04X (Optional)", index);
                        }
                        else
                        {
                            os_snprintf(test_name, sizeof(test_name), "0x%04X:%02X (Optional)", index, sub_index);
                        }

                        result.test_name = test_name;
                        result.class_name = "ExtendedDB_Optional";
                        result.package = "CANopenTerm.Conformance";
                        result.time = 0.0f;
                        result.call_stack = NULL;

                        /* Check if sub-index exists. */
                        read_succeeded = (sdo_state == IS_READ_EXPEDITED || sdo_state == IS_READ_SEGMENTED || sdo_state == IS_READ_BLOCK);

                        if (! read_succeeded && sdo_state != ABORT_TRANSFER)
                        {
                            /* Optional sub-index doesn't exist; this is acceptable. */
                            result.has_passed = true;
                            result.error_type = "OptionalNotAvailable";
                            result.error_message = "Optional object not available";
                            passed_count++;
                            test_add_result(&result);
                        }
                        else if (! read_succeeded && sdo_state == ABORT_TRANSFER)
                        {
                            /* Optional sub-index doesn't exist (abort); this is acceptable. */
                            uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                            const char* abort_desc;
                            abort_code = os_swap_be_32(abort_code);
                            abort_desc = sdo_lookup_abort_code(abort_code);

                            result.has_passed = true;
                            result.error_type = "OptionalNotImplemented";
                            result.error_message = abort_desc;
                            passed_count++;
                            test_add_result(&result);
                        }
                        else
                        {
                            /* Sub-index exists; test both read and write compliance. */
                            bool read_should_succeed = (obj_info.access_type == CONST_T || obj_info.access_type == RO || obj_info.access_type == RW);
                            bool read_compliant = (read_succeeded == read_should_succeed);

                            bool write_compliant;
                            const char* write_failure_reason = NULL;

                            sdo_state_t write_state;

                            uint32 test_value;
                            uint32 write_size;

                            bool write_should_succeed;
                            bool write_succeeded;

                            bool is_compliant;

                            if (obj_info.access_type == WO && sdo_state == ABORT_TRANSFER)
                            {
                                read_compliant = true; /* WO correctly rejected read. */
                            }

                            /* Check write compliance (only if sub-index exists). */
                            write_compliant = true;

                            test_value = 0;
                            write_size = codb_get_data_type_size(obj_info.data_type);
                            if (write_size == 0)
                            {
                                write_size = sizeof(test_value); /* Default to 4 bytes for variable-length types. */
                            }
                            write_state = sdo_write(&sdo_response, SILENT, node_id, index, sub_index, write_size, &test_value, desc);

                            write_should_succeed = (obj_info.access_type == RW || obj_info.access_type == WO);
                            write_succeeded = (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK);
                            write_compliant = (write_succeeded == write_should_succeed);

                            if (! write_compliant)
                            {
                                if (write_state == ABORT_TRANSFER)
                                {
                                    uint32 abort_code = (sdo_response.data[4] << 24) | (sdo_response.data[5] << 16) | (sdo_response.data[6] << 8) | sdo_response.data[7];
                                    abort_code = os_swap_be_32(abort_code);
                                    write_failure_reason = sdo_lookup_abort_code(abort_code);
                                }
                                else if (write_state == IS_WRITE_EXPEDITED || write_state == IS_WRITE_SEGMENTED || write_state == IS_WRITE_BLOCK)
                                {
                                    write_failure_reason = "Write succeeded but should have been rejected";
                                }
                                else
                                {
                                    write_failure_reason = "Write operation not supported";
                                }
                            }

                            /* Determine overall result. */
                            is_compliant = read_compliant && write_compliant;
                            result.has_passed = is_compliant;

                            if (is_compliant)
                            {
                                result.error_type = NULL;
                                result.error_message = NULL;
                                passed_count++;
                            }
                            else
                            {
                                result.error_type = "AccessTypeViolation";
                                if (! read_compliant && ! write_compliant)
                                {
                                    os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed, Write failed (%s)",
                                                access_type_lookup[obj_info.access_type], write_failure_reason);
                                }
                                else if (! read_compliant)
                                {
                                    os_snprintf(error_msg, sizeof(error_msg), "%s object: Read failed",
                                                access_type_lookup[obj_info.access_type]);
                                }
                                else
                                {
                                    os_snprintf(error_msg, sizeof(error_msg), "%s object: Write failed (%s)",
                                                access_type_lookup[obj_info.access_type], write_failure_reason);
                                }
                                result.error_message = error_msg;
                                failed_count++;
                            }

                            /* Only output if test failed. */
                            if (! result.has_passed)
                            {
                                const char* indent = (sub_index == 0) ? "" : "    ";
                                os_printf("  %s[!!] 0x%04X:%02X - %s: %s\n", indent, index, sub_index, obj_info.sub_index_name, result.error_message);
                            }
                            test_add_result(&result);
                        }
                    }
                }
            }
        }
    }

    os_printf("\n");
    os_printf("================================================================================\n");
    os_printf("Test Results Summary:\n");
    os_printf("  Total Tests:   %u\n", test_count);
    os_printf("  Passed:        %u\n", passed_count);
    os_printf("  Failed:        %u\n", failed_count);

    if (test_count > 0)
    {
        float pass_rate = (float)passed_count / (float)test_count * 100.0f;
        os_printf("  Pass Rate:     %.1f%%\n", pass_rate);
    }

    os_printf("================================================================================\n");

    /* Generate test report if file path specified. */
    if (report_file != NULL)
    {
        status = test_generate_report(report_file);
        if (status == ALL_OK)
        {
            os_printf("\nTest report saved to: %s\n", report_file);
        }
        else
        {
            os_log(LOG_ERROR, "Failed to generate test report: %s", report_file);
        }
    }

    os_printf("\n");

    return status;
}
