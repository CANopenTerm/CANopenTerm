/** @file sdo_client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SDO_CLIENT_H
#define SDO_CLIENT_H

#include "SDL.h"
#include "can.h"
#include "core.h"

typedef enum
{
    EXPEDITED_SDO_READ = 0,
    EXPEDITED_SDO_WRITE

} sdo_type_t;

typedef enum
{
    UNSIGNED8  = 1,
    UNSIGNED16 = 2,
    UNSIGNED32 = 4

} data_type_t;

typedef enum
{
    READ_DICT_OBJECT       = 0x40,
    READ_DICT_4_BYTE_SENT  = 0x43, // Read Dictionary Object reply,  expedited, 4 bytes sent
    READ_DICT_3_BYTE_SENT  = 0x47, // Read Dictionary Object reply,  expedited, 3 bytes sent
    READ_DICT_2_BYTE_SENT  = 0x4b, // Read Dictionary Object reply,  expedited, 2 bytes sent
    READ_DICT_1_BYTE_SENT  = 0x4f, // Read Dictionary Object reply,  expedited, 1 byte sent
    WRITE_DICT_OBJECT      = 0x60,
    WRITE_DICT_4_BYTE_SENT = 0x23, // Write Dictionary Object reply, expedited, 4 bytes sent
    WRITE_DICT_3_BYTE_SENT = 0x27, // Write Dictionary Object reply, expedited, 3 bytes sent
    WRITE_DICT_2_BYTE_SENT = 0x2b, // Write Dictionary Object reply, expedited, 2 bytes sent
    WRITE_DICT_1_BYTE_SENT = 0x2f, // Write Dictionary Object reply, expedited, 1 byte sent
    SDO_ABORT              = 0x80

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

Uint32 sdo_read(can_message_t* sdo_response, SDL_bool show_output, Uint8 node_id, Uint16 index, Uint8 sub_index);
Uint32 sdo_write(can_message_t* sdo_response, SDL_bool show_output, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, Uint32 data);
int    lua_sdo_read(lua_State* L);
int    lua_sdo_write(lua_State* L);
void   lua_register_sdo_commands(core_t* core);

#endif /* SDO_CLIENT_H */
