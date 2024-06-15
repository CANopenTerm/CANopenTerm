/** @file sdo_client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SDO_CLIENT_H
#define SDO_CLIENT_H

#include "SDL.h"
#include "lua.h"
#include "can.h"
#include "core.h"

#define DOWNLOAD_RESPONSE_1       0x20
#define DOWNLOAD_RESPONSE_2       0x30
#define UPLOAD_SEGMENT_REQUEST_1  0x60
#define UPLOAD_SEGMENT_REQUEST_2  0x70
#define UPLOAD_SEGMENT_CONTINUE_1 0x00
#define UPLOAD_SEGMENT_CONTINUE_2 0x10
#define ABORT_TRANSFER            0x80

typedef enum
{
    SDO_READ = 0,
    EXPEDITED_SDO_WRITE,
    NORMAL_SDO_WRITE

} sdo_type_t;

typedef enum
{
    UNSIGNED8  = 1,
    UNSIGNED16 = 2,
    UNSIGNED32 = 4

} data_type_t;

typedef enum
{
    UPLOAD_RESPONSE_NORMAL_NO_SIZE      = 0x40, // Upload response, normal transfer, no size indicated
    UPLOAD_RESPONSE_NORMAL_SIZE_IN_DATA = 0x41, // Upload response, normal transfer, size in data
    UPLOAD_RESPONSE_EXPEDIDED_NO_SIZE   = 0x42, // Upload response, expedided transfer, no size indicated
    UPLOAD_RESPONSE_EXPEDIDED_4_BYTE    = 0x43, // Upload response, expedided transfer, 4 byte data
    UPLOAD_RESPONSE_EXPEDIDED_3_BYTE    = 0x47, // Upload response, expedided transfer, 3 byte data
    UPLOAD_RESPONSE_EXPEDIDED_2_BYTE    = 0x4b, // Upload response, expedided transfer, 2 byte data
    UPLOAD_RESPONSE_EXPEDIDED_1_BYTE    = 0x4f, // Upload response, expedided transfer, 1 byte data
    DOWNLOAD_INIT_NORMAL_NO_SIZE        = 0x20, // Download initiate, normal transfer, no size indicated
    DOWNLOAD_INIT_NORMAL_SIZE_IN_DATA   = 0x21, // Download initiate, normal transfer, size in data
    DOWNLOAD_INIT_EXPEDIDED_NO_SIZE     = 0x22, // Download initiate, expedided transfer, no size indicated
    DOWNLOAD_INIT_EXPEDITED_4_BYTE      = 0x23, // Download initiate, expedided transfer, 4 byte data
    DOWNLOAD_INIT_EXPEDITED_3_BYTE      = 0x27, // Download initiate, expedided transfer, 3 byte data
    DOWNLOAD_INIT_EXPEDITED_2_BYTE      = 0x2b, // Download initiate, expedided transfer, 2 byte data
    DOWNLOAD_INIT_EXPEDITED_1_BYTE      = 0x2f, // Download initiate, expedided transfer, 1 byte data

} sdo_command_code_t;

typedef enum
{
    ABORT_TOGGLE_BIT_NOT_ALTERED             = 0x05030000,
    ABORT_SDO_PROTOCOL_TIMED_OUT             = 0x05040000,
    ABORT_CMD_SPECIFIER_INVALID_UNKNOWN      = 0x05040001,
    ABORT_INVALID_BLOCK_SIZE                 = 0x05040002,
    ABORT_INVALID_SEQUENCE_NUMBER            = 0x05040003,
    ABORT_CRC_ERROR                          = 0x05040004,
    ABORT_OUT_OF_MEMORY                      = 0x05040005,
    ABORT_UNSUPPORTED_ACCESS                 = 0x06010000,
    ABORT_ATTEMPT_TO_READ_WRITE_ONLY         = 0x06010001,
    ABORT_ATTEMPT_TO_WRITE_READ_ONLY         = 0x06010002,
    ABORT_OBJECT_DOES_NOT_EXIST              = 0x06020000,
    ABORT_OBJECT_CANNOT_BE_MAPPED            = 0x06040041,
    ABORT_WOULD_EXCEED_PDO_LENGTH            = 0x06040042,
    ABORT_GENERAL_INCOMPATIBILITY_REASON     = 0x06040043,
    ABORT_GENERAL_INTERNAL_INCOMPATIBILITY   = 0x06040047,
    ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR   = 0x06060000,
    ABORT_DATA_TYPE_DOES_NOT_MATCH           = 0x06070010,
    ABORT_DATA_TYPE_LENGTH_TOO_HIGH          = 0x06070012,
    ABORT_DATA_TYPE_LENGTH_TOO_LOW           = 0x06070013,
    ABORT_SUB_INDEX_DOES_NOT_EXIST           = 0x06090011,
    ABORT_INVALID_VALUE_FOR_PARAMETER        = 0x06090030,
    ABORT_VALUE_FOR_PARAMETER_TOO_HIGH       = 0x06090031,
    ABORT_VALUE_FOR_PARAMETER_TOO_LOW        = 0x06090032,
    ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE      = 0x06090036,
    ABORT_RESOURCE_NOT_AVAILABLE             = 0x060a0023,
    ABORT_GENERAL_ERROR                      = 0x08000000,
    ABORT_DATA_CANNOT_BE_TRANSFERRED         = 0x08000020,
    ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL = 0x08000021,
    ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE  = 0x08000022,
    ABORT_NO_OBJECT_DICTIONARY_PRESENT       = 0x08000023,
    ABORT_NO_DATA_AVAILABLE                  = 0x08000024

} sdo_abort_code_t;

Uint32 sdo_read(can_message_t* sdo_response, disp_mode_t disp_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, const char* comment);
Uint32 sdo_write(can_message_t* sdo_response, disp_mode_t disp_mode, sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void* data, const char* comment);
int    lua_sdo_read(lua_State* L);
int    lua_sdo_write(lua_State* L);
void   lua_register_sdo_commands(core_t* core);

#endif /* SDO_CLIENT_H */
