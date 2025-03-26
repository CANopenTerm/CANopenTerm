/** @file sdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "sdo.h"
#include "can.h"
#include "core.h"
#include "dict.h"
#include "os.h"

#define SEGMENT_DATA_SIZE 7u
#define MAX_SDO_RESPONSE_SIZE 8u
#define CAN_BASE_ID 0x600
#define SDO_TIMEOUT_IN_MS 100u

static void print_error(const char* reason, sdo_state_t sdo_state, uint8 node_id, uint16 index, uint8 sub_index, const char* comment, disp_mode_t disp_mode);
static void print_read_result(uint8 node_id, uint16 index, uint8 sub_index, can_message_t* sdo_response, disp_mode_t disp_mode, sdo_state_t sdo_state, const char* comment);
static void print_write_result(sdo_state_t sdo_state, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, disp_mode_t disp_mode, const char* comment);
static int wait_for_response(uint8 node_id, can_message_t* msg_in);

bool is_printable_string(const char* str, size_t length);

const char* sdo_lookup_abort_code(uint32 abort_code)
{
    switch (abort_code)
    {
        case ABORT_TOGGLE_BIT_NOT_ALTERED:
            return "Toggle bit not altered";
        case ABORT_SDO_PROTOCOL_TIMED_OUT:
            return "SDO protocol timed out";
        case ABORT_CMD_SPECIFIER_INVALID_UNKNOWN:
            return "Client/server command specifier not valid or unknown";
        case ABORT_INVALID_BLOCK_SIZE:
            return "Invalid block size";
        case ABORT_INVALID_SEQUENCE_NUMBER:
            return "Invalid sequence number";
        case ABORT_CRC_ERROR:
            return "CRC error";
        case ABORT_OUT_OF_MEMORY:
            return "Out of memory";
        case ABORT_UNSUPPORTED_ACCESS:
            return "Unsupported access to an object";
        case ABORT_ATTEMPT_TO_READ_WRITE_ONLY:
            return "Attempt to read a write only object";
        case ABORT_ATTEMPT_TO_WRITE_READ_ONLY:
            return "Attempt to write a read only object";
        case ABORT_OBJECT_DOES_NOT_EXIST:
            return "Object does not exist in the object dictionary";
        case ABORT_OBJECT_CANNOT_BE_MAPPED:
            return "Object cannot be mapped to the PDO";
        case ABORT_WOULD_EXCEED_PDO_LENGTH:
            return "Number, length of the object would exceed PDO length";
        case ABORT_GENERAL_INCOMPATIBILITY_REASON:
            return "General parameter incompatibility reason";
        case ABORT_GENERAL_INTERNAL_INCOMPATIBILITY:
            return "General internal incompatibility in the device";
        case ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR:
            return "Access failed due to an hardware error";
        case ABORT_DATA_TYPE_DOES_NOT_MATCH:
            return "Data type does not match, length does not match";
        case ABORT_DATA_TYPE_LENGTH_TOO_HIGH:
            return "Data type does not match, length too high";
        case ABORT_DATA_TYPE_LENGTH_TOO_LOW:
            return "Data type does not match, length too low";
        case ABORT_SUB_INDEX_DOES_NOT_EXIST:
            return "Sub-index does not exist";
        case ABORT_INVALID_VALUE_FOR_PARAMETER:
            return "Invalid value for parameter";
        case ABORT_VALUE_FOR_PARAMETER_TOO_HIGH:
            return "Value for parameter written too high";
        case ABORT_VALUE_FOR_PARAMETER_TOO_LOW:
            return "Value for parameter written too low";
        case ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE:
            return "Maximum value is less than minimum value";
        case ABORT_RESOURCE_NOT_AVAILABLE:
            return "Resource not available: SDO connection";
        case ABORT_GENERAL_ERROR:
            return "General error";
        case ABORT_DATA_CANNOT_BE_TRANSFERRED:
            return "Data cannot be transferred";
        case ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL:
            return "Data cannot be transferred or stored to the application because of local control";
        case ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE:
            return "Data cannot be transferred because of the present device state";
        case ABORT_NO_OBJECT_DICTIONARY_PRESENT:
            return "Object dictionary dynamic generation fails or no object dictionary present";
        case ABORT_NO_DATA_AVAILABLE:
            return "No data available";
        default:
            return "Unknown abort code";
    }
}

sdo_state_t sdo_read(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, const char* comment)
{
    can_message_t msg_in = {0};
    can_message_t msg_out = {0};
    char reason[300] = {0};
    sdo_state_t sdo_state = IS_READ_EXPEDITED;
    uint32 abort_code = 0;
    uint32 can_status = 0;

    limit_node_id(&node_id);

    msg_out.id = CAN_BASE_ID + node_id;
    msg_out.data[0] = UPLOAD_RESPONSE_SEGMENT_NO_SIZE;
    msg_out.data[1] = (uint8)(index & 0x00ff);
    msg_out.data[2] = (uint8)((index & 0xff00) >> 8);
    msg_out.data[3] = sub_index;
    msg_out.length = 8;

    can_read(&msg_in); /* Clear buffer. */
    os_memset(&msg_in, 0, sizeof(msg_in));

    can_status = can_write(&msg_out, SILENT, NULL);
    if (0 != can_status)
    {
        print_error(can_get_error_message(can_status), IS_READ_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
        return ABORT_TRANSFER;
    }

    os_memset(&msg_in, 0, sizeof(msg_in));
    while (((index & 0x00ff) != msg_in.data[1]) || (((index & 0xff00) >> 8) != msg_in.data[2]))
    {
        if (0 != wait_for_response(node_id, &msg_in))
        {
            print_error(reason, IS_READ_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }
    }

    switch (msg_in.data[0])
    {
        case UPLOAD_RESPONSE_SEGMENT_NO_SIZE:
        case UPLOAD_RESPONSE_SEGMENT_SIZE_IN_DATA:
            sdo_response->length = msg_in.data[4];
            sdo_state = IS_READ_SEGMENTED;
            break;
        case UPLOAD_RESPONSE_EXPEDITED_4_BYTE:
        case DOWNLOAD_INIT_EXPEDITED_4_BYTE:
            sdo_response->length = 4;
            break;
        case UPLOAD_RESPONSE_EXPEDITED_3_BYTE:
        case DOWNLOAD_INIT_EXPEDITED_3_BYTE:
            sdo_response->length = 3;
            break;
        case UPLOAD_RESPONSE_EXPEDITED_2_BYTE:
        case DOWNLOAD_INIT_EXPEDITED_2_BYTE:
            sdo_response->length = 2;
            break;
        case UPLOAD_RESPONSE_EXPEDITED_1_BYTE:
        case DOWNLOAD_INIT_EXPEDITED_1_BYTE:
            sdo_response->length = 1;
            break;
        default:
        case ABORT_TRANSFER: /* Error. */
            abort_code = (abort_code & 0xffffff00) | msg_in.data[SEGMENT_DATA_SIZE];
            abort_code = (abort_code & 0xffff00ff) | ((uint32)msg_in.data[6] << 8);
            abort_code = (abort_code & 0xff00ffff) | ((uint32)msg_in.data[5] << 16);
            abort_code = (abort_code & 0x00ffffff) | ((uint32)msg_in.data[4] << 24);
            abort_code = os_swap_be_32(abort_code);

            os_snprintf(reason, 300, "0x%08x: %s", abort_code, sdo_lookup_abort_code((abort_code)));
            print_error(reason, IS_READ_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
    }

    if (IS_READ_SEGMENTED == sdo_state)
    {
        int n;
        uint8 cmd = UPLOAD_SEGMENT_REQUEST_1;
        uint8 response_index = 0;
        uint32 data_length = sdo_response->length;
        uint8 remainder = data_length % SEGMENT_DATA_SIZE;
        uint8 expected_msgs = (data_length / SEGMENT_DATA_SIZE) + (remainder ? 1 : 0);
        uint64 timeout_time;
        uint64 time_a;
        uint64 time_b;
        uint64 delta_time;

        msg_out.id = CAN_BASE_ID + node_id;
        msg_out.length = 8;
        msg_out.data[0] = cmd;

        can_status = can_write(&msg_out, SILENT, NULL);
        if (0 != can_status)
        {
            print_error(can_get_error_message(can_status), IS_READ_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }

        for (n = 0; n < expected_msgs; n += 1)
        {
            bool response_received = false;
            timeout_time = 0;
            time_a = os_get_ticks();

            while ((false == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
            {
                can_read(&msg_in);
                if ((0x580 + node_id) == msg_in.id)
                {
                    int can_msg_index = 0;
                    msg_out.data[0] = cmd;

                    if (0 == (msg_in.data[0] % 2))
                    {
                        if (UPLOAD_SEGMENT_REQUEST_1 == cmd)
                        {
                            cmd = UPLOAD_SEGMENT_REQUEST_2;
                        }
                        else
                        {
                            cmd = UPLOAD_SEGMENT_REQUEST_1;
                        }

                        msg_out.data[0] = cmd;

                        can_status = can_write(&msg_out, SILENT, NULL);
                        if (0 != can_status)
                        {
                            print_error(can_get_error_message(can_status), IS_READ_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
                            return ABORT_TRANSFER;
                        }
                    }

                    for (can_msg_index = 1; can_msg_index <= SEGMENT_DATA_SIZE; can_msg_index += 1)
                    {
                        char printable_char;
                        if (response_index >= data_length)
                        {
                            break;
                        }
                        else if (os_isprint(msg_in.data[can_msg_index]))
                        {
                            printable_char = msg_in.data[can_msg_index];
                        }
                        else
                        {
                            break;
                        }
                        sdo_response->data[response_index] = printable_char;

                        response_index += 1;
                    }

                    response_received = true;
                    continue;
                }

                time_b = time_a;
                time_a = os_get_ticks();

                if (time_a > time_b)
                {
                    delta_time = time_a - time_b;
                }
                else
                {
                    delta_time = time_b - time_a;
                }
                timeout_time += delta_time;
            }

            if (timeout_time >= SDO_TIMEOUT_IN_MS)
            {
                os_snprintf(reason, 300, "SDO timeout: CAN-dongle present?");
                print_error(reason, IS_READ_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
                return ABORT_TRANSFER;
            }
        }
    }
    else /* Expedited SDO. */
    {
        os_memcpy(&sdo_response->data, &msg_in.data[4], sdo_response->length);
    }

    print_read_result(node_id, index, sub_index, sdo_response, disp_mode, sdo_state, comment);
    return sdo_state;
}

sdo_state_t sdo_write(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, const char* comment)
{
    can_message_t msg_in = {0};
    can_message_t msg_out = {0};
    char reason[300] = {0};
    int n;
    uint32 abort_code = 0;
    uint32 can_status;
    uint32 u32_value = 0;
    uint32* u32_data_ptr = (uint32*)data;

    limit_node_id(&node_id);

    if (NULL != u32_data_ptr)
    {
        u32_value = *u32_data_ptr;
    }
    else
    {
        print_error("NULL data pointer", IS_WRITE_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
        return ABORT_TRANSFER;
    }

    msg_out.id = CAN_BASE_ID + node_id;
    msg_out.data[1] = (uint8)(index & 0x00ff);
    msg_out.data[2] = (uint8)((index & 0xff00) >> 8);
    msg_out.data[3] = sub_index;
    msg_out.data[4] = (uint8)(u32_value & 0x000000ff);
    msg_out.data[5] = (uint8)((u32_value & 0x0000ff00) >> 8);
    msg_out.data[6] = (uint8)((u32_value & 0x00ff0000) >> 16);
    msg_out.data[7] = (uint8)((u32_value & 0xff000000) >> 24);
    msg_out.length = 8;

    switch (length)
    {
        case 1:
            msg_out.data[0] = DOWNLOAD_INIT_EXPEDITED_1_BYTE;
            break;
        case 2:
            msg_out.data[0] = DOWNLOAD_INIT_EXPEDITED_2_BYTE;
            break;
        case 3:
            msg_out.data[0] = DOWNLOAD_INIT_EXPEDITED_3_BYTE;
            break;
        case 4:
        default:
            msg_out.data[0] = DOWNLOAD_INIT_EXPEDITED_4_BYTE;
            break;
    }

    can_status = can_write(&msg_out, SILENT, NULL);
    if (0 != can_status)
    {
        print_error(can_get_error_message(can_status), IS_WRITE_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
        return ABORT_TRANSFER;
    }

    os_memset(&msg_in, 0, sizeof(msg_in));
    while (((index & 0x00ff) != msg_in.data[1]) || (((index & 0xff00) >> 8) != msg_in.data[2]))
    {
        if (0 != wait_for_response(node_id, &msg_in))
        {
            print_error(reason, IS_WRITE_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }
    }

    switch (msg_in.data[0])
    {
        case UPLOAD_SEGMENT_REQUEST_1:
        case UPLOAD_SEGMENT_REQUEST_2:
            sdo_response->length = msg_out.length;
            break;
        default:
        case ABORT_TRANSFER: /* Error. */
            abort_code = (abort_code & 0xffffff00) | msg_in.data[SEGMENT_DATA_SIZE];
            abort_code = (abort_code & 0xffff00ff) | ((uint32)msg_in.data[6] << 8);
            abort_code = (abort_code & 0xff00ffff) | ((uint32)msg_in.data[5] << 16);
            abort_code = (abort_code & 0x00ffffff) | ((uint32)msg_in.data[4] << 24);
            abort_code = os_swap_be_32(abort_code);

            os_snprintf(reason, 300, "0x%08x: %s", abort_code, sdo_lookup_abort_code((abort_code)));
            print_error(reason, IS_WRITE_EXPEDITED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
    }

    print_write_result(IS_WRITE_EXPEDITED, node_id, index, sub_index, length, data, disp_mode, comment);

    return IS_WRITE_EXPEDITED;
}

sdo_state_t sdo_write_block(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, const char* filename, const char* comment)
{
    can_message_t msg_in = {0};
    can_message_t msg_out = {0};
    char reason[300] = {0};
    void* data = NULL;
    char* byte_data = NULL;
    FILE* file = NULL;
    long file_size = 0;
    size_t bytes_sent = 0;
    uint32 abort_code = 0;
    uint32 can_status = 0;
    uint8 block_size = 0;
    uint8 segment_number = 1;
    uint8 i;
    uint8 remaining_bytes;
    uint8 final_segment_header;

    if (NULL == filename)
    {
        return ABORT_TRANSFER;
    }

    file = fopen(filename, "rb");
    if (NULL == file)
    {
        return ABORT_TRANSFER;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return ABORT_TRANSFER;
    }
    file_size = ftell(file);
    if (file_size == -1L)
    {
        fclose(file);
        return ABORT_TRANSFER;
    }
    rewind(file);

    data = os_calloc((size_t)file_size, sizeof(char));
    if (NULL == data)
    {
        fclose(file);
        return ABORT_TRANSFER;
    }

    if (fread(data, 1, file_size, file) != file_size)
    {
        os_free(data);
        fclose(file);
        return ABORT_TRANSFER;
    }

    limit_node_id(&node_id);

    msg_out.id = CAN_BASE_ID + node_id;
    msg_out.data[0] = UPLOAD_INIT_BLOCK_NO_CRC_SIZE_IN_DATA;
    msg_out.data[1] = (uint8)(index & 0x00ff);
    msg_out.data[2] = (uint8)((index & 0xff00) >> 8);
    msg_out.data[3] = sub_index;
    msg_out.data[4] = (uint8)(file_size & 0xff);
    msg_out.data[5] = (uint8)((file_size >> 8) & 0xff);
    msg_out.data[6] = (uint8)((file_size >> 16) & 0xff);
    msg_out.data[7] = (uint8)((file_size >> 24) & 0xff);
    msg_out.length = 8;

    can_status = can_write(&msg_out, SILENT, NULL);
    if (0 != can_status)
    {
        print_error(can_get_error_message(can_status), IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
        os_free(data);
        fclose(file);
        return ABORT_TRANSFER;
    }

    os_memset(&msg_in, 0, sizeof(msg_in));
    while (((index & 0x00ff) != msg_in.data[1]) || (((index & 0xff00) >> 8) != msg_in.data[2]))
    {
        if (0 != wait_for_response(node_id, &msg_in))
        {
            print_error(reason, IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
            os_free(data);
            fclose(file);
            return ABORT_TRANSFER;
        }
    }

    switch (msg_in.data[0])
    {
        case BLOCK_DOWNLOAD_RESPONSE_NO_CRC:
        case BLOCK_DOWNLOAD_RESPONSE_CRC:
            block_size = msg_in.data[4];
            break;
        case ABORT_TRANSFER: /* Error. */
            abort_code = (abort_code & 0xffffff00) | msg_in.data[SEGMENT_DATA_SIZE];
            abort_code = (abort_code & 0xffff00ff) | ((uint32)msg_in.data[6] << 8);
            abort_code = (abort_code & 0xff00ffff) | ((uint32)msg_in.data[5] << 16);
            abort_code = (abort_code & 0x00ffffff) | ((uint32)msg_in.data[4] << 24);
            abort_code = os_swap_be_32(abort_code);

            os_snprintf(reason, 300, "0x%08x: %s", abort_code, sdo_lookup_abort_code(abort_code));
            print_error(reason, IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
            os_free(data);
            fclose(file);
            return ABORT_TRANSFER;
    }

    byte_data = (char*)data;

    while (bytes_sent < file_size)
    {
        msg_out.data[0] = segment_number;

        for (i = 0; i < SEGMENT_DATA_SIZE && (bytes_sent + i) < file_size; ++i)
        {
            msg_out.data[i + 1] = byte_data[bytes_sent + i];
        }

        /* Mark last segment of last block. */
        if ((bytes_sent + SEGMENT_DATA_SIZE) >= file_size)
        {
            msg_out.data[0] |= 0x80;
        }

        can_status = can_write(&msg_out, SILENT, NULL);
        if (0 != can_status)
        {
            print_error(can_get_error_message(can_status), IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
            os_free(data);
            fclose(file);
            return ABORT_TRANSFER;
        }

        bytes_sent += SEGMENT_DATA_SIZE;
        segment_number += 1;

        if (block_size < segment_number)
        {
            msg_in.data[0] = 0x00;
            msg_in.data[1] = 0x00;
            msg_in.data[2] = 0x00;

            while ((0xA2 != msg_in.data[0]) || (block_size != msg_in.data[1]))
            {
                if (0 != wait_for_response(node_id, &msg_in))
                {
                    print_error(reason, IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
                    os_free(data);
                    fclose(file);
                    return ABORT_TRANSFER;
                }
            }

            /* Adjust block_size based on receiver's response. */
            if (msg_in.data[2] < block_size)
            {
                block_size = msg_in.data[2];
            }
            segment_number = 1;
        }
    }

    /* Handle the final segment. */
    remaining_bytes = file_size - bytes_sent;

    switch (remaining_bytes)
    {
        case 7:
            final_segment_header = 0xC1;
            break;
        case 6:
            final_segment_header = 0xC5;
            break;
        case 5:
            final_segment_header = 0xC9;
            break;
        case 4:
            final_segment_header = 0xCD;
            break;
        case 3:
            final_segment_header = 0xD1;
            break;
        case 2:
            final_segment_header = 0xD5;
            break;
        case 1:
            final_segment_header = 0xD9;
            break;
        default:
            final_segment_header = 0xDD;
            break;
    }

    msg_out.data[0] = final_segment_header;

    for (i = 0; i < SEGMENT_DATA_SIZE; ++i)
    {
        if (i < remaining_bytes)
        {
            msg_out.data[i + 1] = byte_data[bytes_sent + i];
        }
        else
        {
            msg_out.data[i + 1] = 0x00;
        }
    }

    can_status = can_write(&msg_out, SILENT, NULL);
    if (0 != can_status)
    {
        print_error(can_get_error_message(can_status), IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
        os_free(data);
        fclose(file);
        return ABORT_TRANSFER;
    }

    while (1)
    {
        if (0 != wait_for_response(node_id, &msg_in))
        {
            print_error(reason, IS_WRITE_BLOCK, node_id, index, sub_index, comment, disp_mode);
            os_free(data);
            fclose(file);
            return ABORT_TRANSFER;
        }

        if (0xA1 == msg_in.data[0])
        {
            break;
        }
    }

    os_free(data);
    fclose(file);
    return IS_WRITE_BLOCK;
}

sdo_state_t sdo_write_segmented(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, const char* comment)
{
    can_message_t msg_in = {0};
    can_message_t msg_out = {0};
    char reason[300] = {0};
    int data_index = 0;
    int remaining_length = length;
    uint32 abort_code = 0;
    uint32 can_status = 0;
    uint8 cmd = UPLOAD_SEGMENT_CONTINUE_1;
    int i;

    if (length <= 4)
    {
        return sdo_write(sdo_response, disp_mode, node_id, index, sub_index, length, data, comment);
    }

    limit_node_id(&node_id);

    msg_out.id = CAN_BASE_ID + node_id;
    msg_out.data[0] = DOWNLOAD_INIT_SEGMENT_SIZE_IN_DATA;
    msg_out.data[1] = (uint8)(index & 0x00ff);
    msg_out.data[2] = (uint8)((index & 0xff00) >> 8);
    msg_out.data[3] = sub_index;
    msg_out.data[4] = (uint8)(length & 0x000000ff);
    msg_out.data[5] = (uint8)((length & 0x0000ff00) >> 8);
    msg_out.data[6] = (uint8)((length & 0x00ff0000) >> 16);
    msg_out.data[7] = (uint8)((length & 0xff000000) >> 24);
    msg_out.length = 8;

    can_status = can_write(&msg_out, SILENT, NULL);
    if (0 != can_status)
    {
        print_error(can_get_error_message(can_status), IS_WRITE_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
        return ABORT_TRANSFER;
    }

    os_memset(&msg_in, 0, sizeof(msg_in));
    while (((index & 0x00ff) != msg_in.data[1]) || (((index & 0xff00) >> 8) != msg_in.data[2]))
    {
        if (0 != wait_for_response(node_id, &msg_in))
        {
            print_error(reason, IS_WRITE_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }
    }

    switch (msg_in.data[0])
    {
        case UPLOAD_SEGMENT_REQUEST_1:
        case UPLOAD_SEGMENT_REQUEST_2:
            sdo_response->length = msg_out.length;
            break;
        default:
        case ABORT_TRANSFER: /* Error. */
            abort_code = (abort_code & 0xffffff00) | msg_in.data[SEGMENT_DATA_SIZE];
            abort_code = (abort_code & 0xffff00ff) | ((uint32)msg_in.data[6] << 8);
            abort_code = (abort_code & 0xff00ffff) | ((uint32)msg_in.data[5] << 16);
            abort_code = (abort_code & 0x00ffffff) | ((uint32)msg_in.data[4] << 24);
            abort_code = os_swap_be_32(abort_code);

            os_snprintf(reason, 300, "0x%08x: %s", abort_code, sdo_lookup_abort_code(abort_code));
            print_error(reason, IS_WRITE_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
    }

    msg_out.id = CAN_BASE_ID + node_id;
    msg_out.length = 8;
    msg_out.data[0] = cmd;

    while (msg_out.data[0] != (cmd | 0x01))
    {
        msg_out.data[0] = cmd;

        for (i = 1; i <= 7; i++)
        {
            char* data_str = (char*)data;

            if ((0 == remaining_length) || ((cmd | 0x01) == msg_out.data[0]))
            {
                msg_out.data[i] = 0x00; /* Fill remaining bytes with 0x00. */
            }
            else
            {
                msg_out.data[i] = data_str[data_index];
                data_index++;
                remaining_length--;
            }
        }

        if (0 == remaining_length) /* No more data left to send. */
        {
            msg_out.data[0] = (cmd | 0x01);
        }

        can_status = can_write(&msg_out, SILENT, NULL);
        if (0 != can_status)
        {
            print_error(can_get_error_message(can_status), IS_WRITE_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }

        if ((cmd | 0x01) == msg_out.data[0])
        {
            break;
        }

        if (0 == wait_for_response(node_id, &msg_in))
        {
            msg_out.data[0] = cmd;

            switch (msg_in.data[0])
            {
                case DOWNLOAD_RESPONSE_1:
                    cmd = UPLOAD_SEGMENT_CONTINUE_2;
                    break;
                case DOWNLOAD_RESPONSE_2:
                    cmd = UPLOAD_SEGMENT_CONTINUE_1;
                    break;
            }
        }
        else
        {
            print_error(reason, IS_WRITE_SEGMENTED, node_id, index, sub_index, comment, disp_mode);
            return ABORT_TRANSFER;
        }
    }

    print_write_result(IS_WRITE_SEGMENTED, node_id, index, sub_index, length, data, disp_mode, comment);
    return IS_WRITE_SEGMENTED;
}

bool is_printable_string(const char* str, size_t length)
{
    size_t i;
    for (i = 0; i < length; i++)
    {
        if (0 == os_isprint(str[i]))
        {
            return false;
        }
    }
    return true;
}

static void print_error(const char* reason, sdo_state_t sdo_state, uint8 node_id, uint16 index, uint8 sub_index, const char* comment, disp_mode_t disp_mode)
{
    switch (disp_mode)
    {
        case TERM_MODE:
        {
            const char* description = dict_lookup(index, sub_index);

            if (NULL != description)
            {
                os_log(LOG_INFO, "%s", description);
            }

            switch (sdo_state)
            {
                case IS_READ_EXPEDITED:
                case IS_READ_SEGMENTED:
                    os_log(LOG_ERROR, "Index %x, Sub-index %x: 0 byte(s) read error: %s", index, sub_index, reason);
                    break;
                case IS_WRITE_EXPEDITED:
                case IS_WRITE_SEGMENTED:
                    os_log(LOG_ERROR, "Index %x, Sub-index %x: 0 byte(s) write error: %s", index, sub_index, reason);
                    break;
                default:
                    break;
            }
            break;
        }
        case SCRIPT_MODE:
        {
            int i;
            char buffer[34] = {0};
            color_t color = DARK_YELLOW;

            switch (sdo_state)
            {
                case IS_READ_EXPEDITED:
                    os_print(color, "Read ");
                    os_print(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -       ", node_id, index, sub_index);
                    break;
                case IS_WRITE_EXPEDITED:
                case IS_WRITE_SEGMENTED:
                    color = LIGHT_BLUE;
                    os_print(color, "Write");
                    os_print(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -       ", node_id, index, sub_index);
                    break;
                default:
                    break;
            }

            os_print(LIGHT_RED, "FAIL    ");

            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            os_strlcpy(buffer, comment, 33);
            for (i = os_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            os_print(DARK_MAGENTA, "%s ", buffer);
            os_print(DEFAULT_COLOR, "%s\n", reason);
            break;
        }
        default:
        case SILENT:
            break;
    }
}

static void print_progress_bar(size_t bytes_sent, size_t length)
{
    static int prev_percentage = -1;
    int percentage = (int)((bytes_sent * 100) / length);

    if (percentage != prev_percentage)
    {
        printf("Progress: %d%%\r", percentage);
        fflush(stdout);
        prev_percentage = percentage;
    }
}

static void print_read_result(uint8 node_id, uint16 index, uint8 sub_index, can_message_t* sdo_response, disp_mode_t disp_mode, sdo_state_t sdo_state, const char* comment)
{
    uint32 u32_value = 0;
    char str_buffer[5] = {0};

    if (sdo_response->length >= 4)
    {
        os_memcpy(&u32_value, &sdo_response->data, sizeof(uint32));
        os_memcpy(&str_buffer, &u32_value, sizeof(uint32));
    }
    else
    {
        os_memcpy(&u32_value, &sdo_response->data, sdo_response->length);
        os_memcpy(&str_buffer, &u32_value, sdo_response->length);
    }

    if (false == is_printable_string(str_buffer, sizeof(uint32)))
    {
        str_buffer[0] = '\0';
    }

    switch (disp_mode)
    {
        case TERM_MODE:
        {
            const char* description = dict_lookup(index, sub_index);

            if (NULL != description)
            {
                os_log(LOG_INFO, "%s", description);
            }

            switch (sdo_state)
            {
                case IS_READ_EXPEDITED:
                    os_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) read: %u (0x%x) %s",
                           index,
                           sub_index,
                           sdo_response->length,
                           u32_value,
                           u32_value,
                           str_buffer);
                    break;
                case IS_READ_SEGMENTED:
                    sdo_response->data[CAN_BUF_SIZE - 1] = '\0';
                    os_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) read: %s",
                           index,
                           sub_index,
                           sdo_response->length,
                           (char*)sdo_response->data);
                    break;
                default:
                    return;
            }
            break;
        }
        case SCRIPT_MODE:
        {
            int i;
            char buffer[34] = {0};

            sdo_response->data[CAN_BUF_SIZE - 1] = '\0';

            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            os_memcpy(buffer, comment, 33);
            for (i = os_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            os_print(DARK_YELLOW, "Read     ");
            os_print(DEFAULT_COLOR, "0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, sdo_response->length);
            os_print(LIGHT_GREEN, "SUCC    ");
            os_print(DARK_MAGENTA, "%s ", buffer);

            if (IS_READ_EXPEDITED == sdo_state)
            {
                switch (sdo_response->length)
                {
                    case 4:
                        os_print(DEFAULT_COLOR, "0x%08X %u (U32) %s", u32_value, u32_value, str_buffer);
                        break;
                    case 3:
                        os_print(DEFAULT_COLOR, "0x%06X %u (U24) %s", u32_value, u32_value, str_buffer);
                        break;
                    case 2:
                        os_print(DEFAULT_COLOR, "0x%04X %u (U16) %s", u32_value, u32_value, str_buffer);
                        break;
                    case 1:
                        os_print(DEFAULT_COLOR, "0x%02X %u (U8) %s", u32_value, u32_value, str_buffer);
                        break;
                }
            }
            else /* Read segmented SDO. */
            {
                os_print(DEFAULT_COLOR, "%s", (char*)sdo_response->data);
            }
            puts("");
            break;
        }
        default:
        case SILENT:
            break;
    }
}

void print_write_result(sdo_state_t sdo_state, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, disp_mode_t disp_mode, const char* comment)
{
    uint32 u32_value = 0;
    uint32* u32_data_ptr = (uint32*)data;
    char* data_str = (char*)data;

    if (NULL != u32_data_ptr)
    {
        u32_value = *u32_data_ptr;
    }

    if (NULL == data_str)
    {
        data_str = "";
    }

    switch (disp_mode)
    {
        case TERM_MODE:
        {
            const char* description = dict_lookup(index, sub_index);

            if (NULL != description)
            {
                os_log(LOG_INFO, "%s", description);
            }

            if (IS_WRITE_EXPEDITED == sdo_state)
            {
                char str[5] = {0};

                switch (length)
                {
                    case 1:
                        str[0] = os_isprint(data_str[0]) ? data_str[0] : '\0';
                        break;
                    case 2:
                        str[0] = os_isprint(data_str[0]) ? data_str[0] : '\0';
                        str[1] = os_isprint(data_str[1]) ? data_str[1] : '\0';
                        break;
                    case 3:
                        str[0] = os_isprint(data_str[0]) ? data_str[0] : '\0';
                        str[1] = os_isprint(data_str[1]) ? data_str[1] : '\0';
                        str[2] = os_isprint(data_str[2]) ? data_str[2] : '\0';
                        break;
                    case 4:
                        str[0] = os_isprint(data_str[0]) ? data_str[0] : '\0';
                        str[1] = os_isprint(data_str[1]) ? data_str[1] : '\0';
                        str[2] = os_isprint(data_str[2]) ? data_str[2] : '\0';
                        str[3] = os_isprint(data_str[3]) ? data_str[3] : '\0';
                        break;
                    default:
                        break;
                }

                os_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) written: %u (0x%x) %s",
                       index,
                       sub_index,
                       length,
                       u32_value,
                       u32_value,
                       str);
            }
            else if (IS_WRITE_SEGMENTED)
            {
                data_str[CAN_BUF_SIZE - 1] = '\0';

                os_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) written: %s",
                       index,
                       sub_index,
                       os_strlen(data_str),
                       data_str);
            }
            else
            {
                return;
            }
            break;
        }
        case SCRIPT_MODE:
        {
            int i;
            char buffer[34] = {0};

            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            os_strlcpy(buffer, comment, 33);
            for (i = os_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            os_print(DARK_BLUE, "Write    ");
            os_print(DEFAULT_COLOR, "0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, length);
            os_print(LIGHT_GREEN, "SUCC    ");
            os_print(DARK_MAGENTA, "%s ", buffer);

            if (IS_WRITE_EXPEDITED == sdo_state)
            {
                switch (length)
                {
                    case 4:
                        os_print(DEFAULT_COLOR, "0x%08X %u (U32)", u32_value, u32_value);
                        break;
                    case 3:
                        os_print(DEFAULT_COLOR, "0x%06X %u (U24)", u32_value, u32_value);
                        break;
                    case 2:
                        os_print(DEFAULT_COLOR, "0x%04X %u (U16)", u32_value, u32_value);
                        break;
                    case 1:
                        os_print(DEFAULT_COLOR, "0x%02X %u (U8)", u32_value, u32_value);
                        break;
                }
            }
            else
            {
                os_print(DEFAULT_COLOR, "%s", data_str);
            }
            puts("");

            break;
        }
        case SILENT:
            break;
    }
}

static int wait_for_response(uint8 node_id, can_message_t* msg_in)
{
    uint64 time_a = os_get_ticks();
    uint64 timeout_time = 0;
    bool response_received = false;

    while ((false == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
    {
        uint64 time_b;
        uint64 delta_time;

        can_read(msg_in);
        if ((0x580 + node_id) == msg_in->id)
        {
            response_received = true;
            continue;
        }

        time_b = time_a;
        time_a = os_get_ticks();

        if (time_a > time_b)
        {
            delta_time = time_a - time_b;
        }
        else
        {
            delta_time = time_b - time_a;
        }
        timeout_time += delta_time;
    }

    if (timeout_time >= SDO_TIMEOUT_IN_MS)
    {
        char reason[300] = {0};
        os_snprintf(reason, 300, "SDO timeout: CAN-dongle present?");
        return 1;
    }

    return 0;
}
