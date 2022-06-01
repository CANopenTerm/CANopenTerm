/** @file sdo_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "nuklear.h"
#include "can.h"
#include "core.h"
#include "printf.h"
#include "sdo_client.h"

#define SDO_TIMEOUT_IN_MS 100

static Uint32 sdo_result;

static Uint32 sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, SDL_bool show_result, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, Uint32 data);
static void   print_abort_code_error(Uint32 abort_code);

Uint32 sdo_read(can_message_t* sdo_response, SDL_bool show_result, Uint8 node_id, Uint16 index, Uint8 sub_index)
{
    return sdo_send(
        EXPEDITED_SDO_READ,
        sdo_response,
        show_result,
        node_id,
        index,
        sub_index,
        0, 0);
}

Uint32 sdo_write(can_message_t* sdo_response, SDL_bool show_result, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, Uint32 data)
{
    return sdo_send(
        EXPEDITED_SDO_WRITE,
        sdo_response,
        show_result,
        node_id,
        index,
        sub_index,
        length,
        data);
}

int lua_sdo_read(lua_State* L)
{
    can_message_t sdo_response = { 0 };;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (0 != sdo_read(
            &sdo_response,
            SDL_FALSE,
            (Uint8)node_id,
            (Uint16)index,
            (Uint8)sub_index))
    {
        return 0;
    }
    else
    {
        sdo_result = (Uint32)sdo_response.data[4];
        lua_pushinteger(L, sdo_result);
        lua_setglobal(L, "sdo_result");
        return 1;
    }
}

int lua_sdo_write(lua_State* L)
{
    can_message_t sdo_response = { 0 };
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    int           length       = luaL_checkinteger(L, 4);
    int           data         = luaL_checkinteger(L, 5);

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (0 != sdo_write(
            &sdo_response,
            SDL_FALSE,
            (Uint8)node_id,
            (Uint16)index,
            (Uint8)sub_index,
            (Uint8)length,
            (Uint32)data))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void lua_register_sdo_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_sdo_read);
    lua_setglobal(core->L, "sdo_read");

    lua_pushcfunction(core->L, lua_sdo_write);
    lua_setglobal(core->L, "sdo_write");
}

static Uint32 sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, SDL_bool show_result, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, Uint32 data)
{
    can_message_t can_message       = { 0 };
    Uint32        can_status        = 0;
    SDL_bool      response_received = SDL_FALSE;
    Uint64        timeout_time      = 0;
    Uint64        time_a;

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    can_message.id      = 0x600 + node_id;

    can_message.data[1] = (Uint8)(index  & 0x00ff);
    can_message.data[2] = (Uint8)((index & 0xff00) >> 8);
    can_message.data[3] = sub_index;

    switch (sdo_type)
    {
        default:
        case EXPEDITED_SDO_READ:
            can_message.length  = 8;
            can_message.data[0] = READ_DICT_OBJECT;
            break;
        case EXPEDITED_SDO_WRITE:
            can_message.length  = 4 + length;
            can_message.data[4] = (Uint8)(data  & 0x000000ff);
            can_message.data[5] = (Uint8)((data & 0x0000ff00) >> 8);
            can_message.data[6] = (Uint8)((data & 0x00ff0000) >> 16);
            can_message.data[7] = (Uint8)((data & 0xff000000) >> 24);
            switch(length)
            {
                case 1:
                    can_message.data[0] = WRITE_DICT_1_BYTE_SENT;
                    break;
                case 2:
                    can_message.data[0] = WRITE_DICT_2_BYTE_SENT;
                    break;
                case 3:
                    can_message.data[0] = WRITE_DICT_3_BYTE_SENT;
                    break;
                case 4:
                default:
                    can_message.data[0] = WRITE_DICT_4_BYTE_SENT;
                    break;
            }
            break;
    }

    can_status = can_write(&can_message);
    if (0 != can_status)
    {
        can_print_error_message(NULL, can_status);
    }

    time_a = SDL_GetTicks64();
    while ((SDL_FALSE == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
    {
        Uint64 time_b;
        Uint64 delta_time;

        if ((0x580 + node_id) == can_message.id)
        {
            if ((index  & 0x00ff) == can_message.data[1])
            {
                if (((index & 0xff00) >> 8) == can_message.data[2])
                {
                    response_received = SDL_TRUE;
                    continue;
                }
            }
        }

        can_status = can_read(&can_message);
        time_b     = time_a;
        time_a     = SDL_GetTicks64();

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

    if (0 != can_status)
    {
        can_print_error_message(NULL, can_status);
    }
    else if (timeout_time >= SDO_TIMEOUT_IN_MS)
    {
        c_log(LOG_WARNING, "SDO timeout: USB-dongle present?");
    }
    else
    {
        Uint32 abort_code = 0;
        int    data_index;
        switch (can_message.data[0])
        {
            case READ_DICT_4_BYTE_SENT:
            case WRITE_DICT_4_BYTE_SENT:
                sdo_response->length = 4;
                break;
            case READ_DICT_3_BYTE_SENT:
            case WRITE_DICT_3_BYTE_SENT:
                sdo_response->length = 3;
                break;
            case READ_DICT_2_BYTE_SENT:
            case WRITE_DICT_2_BYTE_SENT:
                sdo_response->length = 2;
                break;
            case READ_DICT_1_BYTE_SENT:
            case WRITE_DICT_1_BYTE_SENT:
                sdo_response->length = 1;
                break;
            case SDO_ABORT:
                abort_code = (abort_code & 0xffffff00) | can_message.data[7];
                abort_code = (abort_code & 0xffff00ff) | ((Uint32)can_message.data[6] << 8);
                abort_code = (abort_code & 0xff00ffff) | ((Uint32)can_message.data[5] << 16);
                abort_code = (abort_code & 0x00ffffff) | ((Uint32)can_message.data[4] << 24);
                abort_code = SDL_SwapBE32(abort_code);

                print_abort_code_error(abort_code);
                return can_status;
        }

        for (data_index = 0; data_index < sdo_response->length; data_index += 1)
        {
            sdo_response->data[4 + data_index] = can_message.data[4 + data_index];
        }

        if (SDL_TRUE == show_result)
        {
            const  char*  str_read    = "read";
            const  char*  str_written = "written";
            const  char** str_action  = NULL;
            Uint32        output_data;

            switch (sdo_type)
            {
                default:
                case EXPEDITED_SDO_READ:
                    output_data = (Uint32)sdo_response->data[4];
                    str_action  = &str_read;
                    break;
                case EXPEDITED_SDO_WRITE:
                    output_data = data;
                    str_action  = &str_written;
                    break;
            }

            c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) %s: %u (0x%x)",
                  index,
                  sub_index,
                  sdo_response->length,
                  *str_action,
                  output_data,
                  output_data);
        }
    }

    return can_status;
}

static void print_abort_code_error(Uint32 abort_code)
{
    switch(abort_code)
    {
        case ABORT_TOGGLE_BIT_NOT_ALTERED:
            c_log(LOG_WARNING, "Toggle bit not altered");
            break;
        case ABORT_SDO_PROTOCOL_TIMED_OUT:
            c_log(LOG_WARNING, "SDO protocol timed out");
            break;
        case ABORT_CMD_SPECIFIER_INVALID_UNKNOWN:
            c_log(LOG_WARNING, "Client/server command specifier not valid or unknown");
            break;
        case ABORT_INVALID_BLOCK_SIZE:
            c_log(LOG_WARNING, "Invalid block size");
            break;
        case ABORT_INVALID_SEQUENCE_NUMBER:
            c_log(LOG_WARNING, "Invalid sequence number");
            break;
        case ABORT_CRC_ERROR:
            c_log(LOG_WARNING, "CRC error");
            break;
        case ABORT_OUT_OF_MEMORY:
            c_log(LOG_WARNING, "Out of memory");
            break;
        case ABORT_UNSUPPORTED_ACCESS:
            c_log(LOG_WARNING, "Unsupported access to an object");
            break;
        case ABORT_ATTEMPT_TO_READ_WRITE_ONLY:
            c_log(LOG_WARNING, "Attempt to read a write only object");
            break;
        case ABORT_ATTEMPT_TO_WRITE_READ_ONLY:
            c_log(LOG_WARNING, "Attempt to write a read only object");
            break;
        case ABORT_OBJECT_DOES_NOT_EXIST:
            c_log(LOG_WARNING, "Object does not exist in the object dictionary");
            break;
        case ABORT_OBJECT_CANNOT_BE_MAPPED:
            c_log(LOG_WARNING, "Object cannot be mapped to the PDO");
            break;
        case ABORT_WOULD_EXCEED_PDO_LENGTH:
            c_log(LOG_WARNING, "Number, length of the object would exceed PDO length");
            break;
        case ABORT_GENERAL_INCOMPATIBILITY_REASON:
            c_log(LOG_WARNING, "General parameter incompatibility reason");
            break;
        case ABORT_GENERAL_INTERNAL_INCOMPATIBILITY:
            c_log(LOG_WARNING, "General internal incompatibility in the device");
            break;
        case ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR:
            c_log(LOG_WARNING, "Access failed due to an hardware error");
            break;
        case ABORT_DATA_TYPE_DOES_NOT_MATCH:
            c_log(LOG_WARNING, "Data type does not match, length does not match");
            break;
        case ABORT_DATA_TYPE_LENGTH_TOO_HIGH:
            c_log(LOG_WARNING, "Data type does not match, length too high");
            break;
        case ABORT_DATA_TYPE_LENGTH_TOO_LOW:
            c_log(LOG_WARNING, "Data type does not match, length too low");
            break;
        case ABORT_SUB_INDEX_DOES_NOT_EXIST:
            c_log(LOG_WARNING, "Sub-index does not exist");
            break;
        case ABORT_INVALID_VALUE_FOR_PARAMETER:
            c_log(LOG_WARNING, "Invalid value for parameter");
            break;
        case ABORT_VALUE_FOR_PARAMETER_TOO_HIGH:
            c_log(LOG_WARNING, "Value for parameter written too high");
            break;
        case ABORT_VALUE_FOR_PARAMETER_TOO_LOW:
            c_log(LOG_WARNING, "Value for parameter written too low");
            break;
        case ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE:
            c_log(LOG_WARNING, "Maximum value is less than minimum value");
            break;
        case ABORT_RESOURCE_NOT_AVAILABLE:
            c_log(LOG_WARNING, "Resource not available: SDO connection");
            break;
        case ABORT_GENERAL_ERROR:
            c_log(LOG_WARNING, "General error");
            break;
        case ABORT_DATA_CANNOT_BE_TRANSFERRED:
            c_log(LOG_WARNING, "Data cannot be transferred");
            break;
        case ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL:
            c_log(LOG_WARNING, "Data cannot be transferred or stored to the application because of local control");
            break;
        case ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE:
            c_log(LOG_WARNING, "Data cannot be transferred because of the present device state");
            break;
        case ABORT_NO_OBJECT_DICTIONARY_PRESENT:
            c_log(LOG_WARNING, "Object dictionary dynamic generation fails or no object dictionary present");
            break;
        case ABORT_NO_DATA_AVAILABLE:
            c_log(LOG_WARNING, "No data available");
            break;
        default:
            c_log(LOG_WARNING, "Unknown abort code: 0x%x", abort_code);
    }
}
