/** @file sdo.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SDO_H
#define SDO_H

#include "can.h"
#include "core.h"

#define DOWNLOAD_RESPONSE_1 0x20
#define DOWNLOAD_RESPONSE_2 0x30
#define UPLOAD_SEGMENT_REQUEST_1 0x60
#define UPLOAD_SEGMENT_REQUEST_2 0x70
#define UPLOAD_SEGMENT_CONTINUE_1 0x00
#define UPLOAD_SEGMENT_CONTINUE_2 0x10
#define BLOCK_DOWNLOAD_RESPONSE_NO_CRC 0xa0
#define BLOCK_DOWNLOAD_RESPONSE_CRC 0xa4

typedef enum
{
    IS_READ_EXPEDITED = 0,
    IS_READ_SEGMENTED,
    IS_READ_BLOCK,
    IS_WRITE_EXPEDITED,
    IS_WRITE_SEGMENTED,
    IS_WRITE_BLOCK,
    ABORT_TRANSFER = 0x80

} sdo_state_t;

typedef enum
{
    UPLOAD_RESPONSE_SEGMENT_NO_SIZE = 0x40,       /* Upload response, segment transfer, no size indicated */
    UPLOAD_RESPONSE_SEGMENT_SIZE_IN_DATA = 0x41,  /* Upload response, segment transfer, size in data */
    UPLOAD_RESPONSE_EXPEDITED_NO_SIZE = 0x42,     /* Upload response, EXPEDITED transfer, no size indicated */
    UPLOAD_RESPONSE_EXPEDITED_4_BYTE = 0x43,      /* Upload response, EXPEDITED transfer, 4 byte data */
    UPLOAD_RESPONSE_EXPEDITED_3_BYTE = 0x47,      /* Upload response, EXPEDITED transfer, 3 byte data */
    UPLOAD_RESPONSE_EXPEDITED_2_BYTE = 0x4b,      /* Upload response, EXPEDITED transfer, 2 byte data */
    UPLOAD_RESPONSE_EXPEDITED_1_BYTE = 0x4f,      /* Upload response, EXPEDITED transfer, 1 byte data */
    DOWNLOAD_INIT_SEGMENT_NO_SIZE = 0x20,         /* Download initiate, segment transfer, no size indicated */
    DOWNLOAD_INIT_SEGMENT_SIZE_IN_DATA = 0x21,    /* Download initiate, segment transfer, size in data */
    DOWNLOAD_INIT_EXPEDITED_NO_SIZE = 0x22,       /* Download initiate, EXPEDITED transfer, no size indicated */
    DOWNLOAD_INIT_EXPEDITED_4_BYTE = 0x23,        /* Download initiate, EXPEDITED transfer, 4 byte data */
    DOWNLOAD_INIT_EXPEDITED_3_BYTE = 0x27,        /* Download initiate, EXPEDITED transfer, 3 byte data */
    DOWNLOAD_INIT_EXPEDITED_2_BYTE = 0x2b,        /* Download initiate, EXPEDITED transfer, 2 byte data */
    DOWNLOAD_INIT_EXPEDITED_1_BYTE = 0x2f,        /* Download initiate, EXPEDITED transfer, 1 byte data */
    UPLOAD_INIT_BLOCK_NO_CRC_NO_SIZE = 0xc0,      /* Upload initiate, block transfer, no CRC, no size indicated */
    UPLOAD_INIT_BLOCK_NO_CRC_SIZE_IN_DATA = 0xc2, /* Upload initiate, block transfer, no CRC, size in data */
    UPLOAD_INIT_BLOCK_CRC_NO_SIZE = 0xc4,         /* Upload initiate, block transfer, CRC, no size indicated */
    UPLOAD_INIT_BLOCK_CRC_SIZE_IN_DATA = 0xC6     /* Upload initiate, block transfer, CRC, size in data */

} sdo_command_code_t;

typedef enum
{
    ABORT_TOGGLE_BIT_NOT_ALTERED = 0x05030000,
    ABORT_SDO_PROTOCOL_TIMED_OUT = 0x05040000,
    ABORT_CMD_SPECIFIER_INVALID_UNKNOWN = 0x05040001,
    ABORT_INVALID_BLOCK_SIZE = 0x05040002,
    ABORT_INVALID_SEQUENCE_NUMBER = 0x05040003,
    ABORT_CRC_ERROR = 0x05040004,
    ABORT_OUT_OF_MEMORY = 0x05040005,
    ABORT_UNSUPPORTED_ACCESS = 0x06010000,
    ABORT_ATTEMPT_TO_READ_WRITE_ONLY = 0x06010001,
    ABORT_ATTEMPT_TO_WRITE_READ_ONLY = 0x06010002,
    ABORT_OBJECT_DOES_NOT_EXIST = 0x06020000,
    ABORT_OBJECT_CANNOT_BE_MAPPED = 0x06040041,
    ABORT_WOULD_EXCEED_PDO_LENGTH = 0x06040042,
    ABORT_GENERAL_INCOMPATIBILITY_REASON = 0x06040043,
    ABORT_GENERAL_INTERNAL_INCOMPATIBILITY = 0x06040047,
    ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR = 0x06060000,
    ABORT_DATA_TYPE_DOES_NOT_MATCH = 0x06070010,
    ABORT_DATA_TYPE_LENGTH_TOO_HIGH = 0x06070012,
    ABORT_DATA_TYPE_LENGTH_TOO_LOW = 0x06070013,
    ABORT_SUB_INDEX_DOES_NOT_EXIST = 0x06090011,
    ABORT_INVALID_VALUE_FOR_PARAMETER = 0x06090030,
    ABORT_VALUE_FOR_PARAMETER_TOO_HIGH = 0x06090031,
    ABORT_VALUE_FOR_PARAMETER_TOO_LOW = 0x06090032,
    ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE = 0x06090036,
    ABORT_RESOURCE_NOT_AVAILABLE = 0x060a0023,
    ABORT_GENERAL_ERROR = 0x08000000,
    ABORT_DATA_CANNOT_BE_TRANSFERRED = 0x08000020,
    ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL = 0x08000021,
    ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE = 0x08000022,
    ABORT_NO_OBJECT_DICTIONARY_PRESENT = 0x08000023,
    ABORT_NO_DATA_AVAILABLE = 0x08000024

} sdo_abort_code_t;

const char* sdo_lookup_abort_code(uint32 abort_code);
sdo_state_t sdo_read(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, const char* comment);
sdo_state_t sdo_write(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, const char* comment);
sdo_state_t sdo_write_block(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, const char* filename, const char* comment);
sdo_state_t sdo_write_segmented(can_message_t* sdo_response, disp_mode_t disp_mode, uint8 node_id, uint16 index, uint8 sub_index, uint32 length, void* data, const char* comment);

#endif /* SDO_H */
