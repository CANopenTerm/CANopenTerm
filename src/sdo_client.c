/** @file sdo_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "lua.h"
#include "lauxlib.h"
#include "can.h"
#include "core.h"
#include "dict.h" 
#include "printf.h"
#include "sdo_client.h"

#define SDO_TIMEOUT_IN_MS 100

static Uint32      sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, disp_mode_t disp_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void *data, const char* comment);
static const char* lookup_abort_code(Uint32 abort_code);
static void        print_error(const char* reason, sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, const char* comment, disp_mode_t disp_mode);
void               print_read_result(Uint8 node_id, Uint16 index, Uint8 sub_index, can_message_t* sdo_response, disp_mode_t disp_mode, SDL_bool is_normal_sdo, const char* comment);
void               print_write_result(sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void* data, disp_mode_t disp_mode, const char* comment);

Uint32 sdo_read(can_message_t* sdo_response,disp_mode_t disp_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, const char* comment)
{
    return sdo_send(
        SDO_READ,
        sdo_response,
        disp_mode,
        node_id,
        index,
        sub_index,
        0, 0,
        comment);
}

Uint32 sdo_write(can_message_t* sdo_response, disp_mode_t disp_mode, sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void *data, const char* comment)
{
    return sdo_send(
        sdo_type,
        sdo_response,
        disp_mode,
        node_id,
        index,
        sub_index,
        length,
        data,
        comment);
}

int lua_sdo_read(lua_State* L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = NO_OUTPUT;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    int           status;
    SDL_bool      show_output  = lua_toboolean(L, 4);
    const char*   comment      = lua_tostring(L, 5);

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (SDL_TRUE == show_output)
    {
        disp_mode = SCRIPT_OUTPUT;
    }

    status = sdo_read(
        &sdo_response,
        disp_mode,
        (Uint8)node_id,
        (Uint16)index,
        (Uint16)sub_index,
        comment);

    switch (status)
    {
        case UPLOAD_RESPONSE_NORMAL_NO_SIZE:
        case UPLOAD_RESPONSE_NORMAL_SIZE_IN_DATA:
            lua_pushstring(L, (const char*)sdo_response.data);
            break;
        case ABORT_TRANSFER:
            lua_pushnil(L);
            break;
        default:
            lua_pushinteger(L, (Uint32)sdo_response.data[4]);
            break;
    }

    return 1;
}

int lua_sdo_write(lua_State* L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = NO_OUTPUT;
    int           status;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    int           length       = luaL_checkinteger(L, 4);
    int           data         = lua_tointeger(L, 5);
    SDL_bool      show_output  = lua_toboolean(L, 6);
    const char*   comment      = lua_tostring(L, 7);

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (SDL_TRUE == show_output)
    {
        disp_mode = SCRIPT_OUTPUT;
    }

    status = sdo_write(
        &sdo_response,
        disp_mode,
        EXPEDITED_SDO_WRITE,
        (Uint8)node_id,
        (Uint16)index,
        (Uint8)sub_index,
        (Uint8)length,
        (void*)&data,
        comment);

    switch (status)
    {
        case ABORT_TRANSFER:
            lua_pushboolean(L, 0);
            break;
        default:
            lua_pushboolean(L, 1);
            break;
    }

    return 1;
}

int lua_sdo_write_string(lua_State* L)
{
    can_message_t sdo_response = { 0 };
    disp_mode_t   disp_mode    = NO_OUTPUT;
    int           status;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    const char*   data         = luaL_checkstring(L, 4);
    Uint8         length       = 0;
    SDL_bool      show_output  = lua_toboolean(L, 5);
    const char*   comment      = lua_tostring(L, 6);

    if (NULL != data)
    {
        length = SDL_strlen(data);
    }
    else
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (SDL_TRUE == show_output)
    {
        disp_mode = SCRIPT_OUTPUT;
    }

    status = sdo_write(
        &sdo_response,
        disp_mode,
        NORMAL_SDO_WRITE,
        (Uint8)node_id,
        (Uint16)index,
        (Uint8)sub_index,
        length,
        (void*)data,
        comment);

    switch (status)
    {
        case ABORT_TRANSFER:
            lua_pushboolean(L, 0);
            break;
        default:
            lua_pushboolean(L, 1);
            break;
    }

    return 1;
}

void lua_register_sdo_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_sdo_read);
    lua_setglobal(core->L, "sdo_read");

    lua_pushcfunction(core->L, lua_sdo_write);
    lua_setglobal(core->L, "sdo_write");

    lua_pushcfunction(core->L, lua_sdo_write_string);
    lua_setglobal(core->L, "sdo_write_string");
}

static Uint32 sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, disp_mode_t disp_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void* data, const char* comment)
{
    can_message_t msg_in            = { 0 };
    can_message_t msg_out           = { 0 };
    SDL_bool      is_normal_sdo     = SDL_FALSE;
    SDL_bool      response_received = SDL_FALSE;
    Uint32        can_status        = 0;
    Uint32        u32_value         = 0;
    Uint32*       u32_data_ptr      = (Uint32*)data;
    Uint64        time_a;
    Uint64        timeout_time      = 0;
    Uint8         command_code      = 0x00;
    char          reason[300]       = { 0 };

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    msg_out.id = 0x600 + node_id;
    msg_out.data[1] = (Uint8)(index & 0x00ff);
    msg_out.data[2] = (Uint8)((index & 0xff00) >> 8);
    msg_out.data[3] = sub_index;

    switch (sdo_type)
    {
        default:
        case SDO_READ:
            msg_out.length  = 8;
            msg_out.data[0] = UPLOAD_RESPONSE_NORMAL_NO_SIZE;
            break;
        case EXPEDITED_SDO_WRITE:
        {
            if (NULL != u32_data_ptr)
            {
                u32_value = *u32_data_ptr;
            }

            msg_out.length  = 4 + length;
            msg_out.data[4] = (Uint8)(u32_value & 0x000000ff);
            msg_out.data[5] = (Uint8)((u32_value & 0x0000ff00) >> 8);
            msg_out.data[6] = (Uint8)((u32_value & 0x00ff0000) >> 16);
            msg_out.data[7] = (Uint8)((u32_value & 0xff000000) >> 24);

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
            break;
        }
        case NORMAL_SDO_WRITE:
        {
            msg_out.length  = 8;
            msg_out.data[0] = DOWNLOAD_INIT_NORMAL_SIZE_IN_DATA;
            msg_out.data[4] = length;
            break;
        }
    }

    can_status = can_write(&msg_out, NO_OUTPUT, NULL);
    if (0 != can_status)
    {
        /* Nothing to do here. */
    }

    time_a = SDL_GetTicks64();
    while ((SDL_FALSE == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
    {
        Uint64 time_b;
        Uint64 delta_time;
    
        can_read(&msg_in);
        if ((0x580 + node_id) == msg_in.id)
        {
            if ((index & 0x00ff) == msg_in.data[1])
            {
                if (((index & 0xff00) >> 8) == msg_in.data[2])
                {
                    response_received = SDL_TRUE;
                    continue;
                }
            }
        }
    
        time_b = time_a;
        time_a = SDL_GetTicks64();
    
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

    command_code = msg_in.data[0];

    if (timeout_time >= SDO_TIMEOUT_IN_MS)
    {
        SDL_snprintf(reason, 300, "SDO timeout: CAN-dongle present?");
        print_error(reason, sdo_type, node_id, index, sub_index, comment, disp_mode);
        return ABORT_TRANSFER;
    }
    else
    {
        Uint32 abort_code = 0;
        int    n;

        switch (command_code)
        {
            case UPLOAD_SEGMENT_REQUEST_1:
            case UPLOAD_SEGMENT_REQUEST_2:
                sdo_response->length = msg_out.length;
                break;
            case UPLOAD_RESPONSE_NORMAL_NO_SIZE:
            case UPLOAD_RESPONSE_NORMAL_SIZE_IN_DATA:
                sdo_response->length = msg_in.data[4];
                is_normal_sdo        = SDL_TRUE;
                break;
            case UPLOAD_RESPONSE_EXPEDIDED_4_BYTE:
            case DOWNLOAD_INIT_EXPEDITED_4_BYTE:
                sdo_response->length = 4;
                break;
            case UPLOAD_RESPONSE_EXPEDIDED_3_BYTE:
            case DOWNLOAD_INIT_EXPEDITED_3_BYTE:
                sdo_response->length = 3;
                break;
            case UPLOAD_RESPONSE_EXPEDIDED_2_BYTE:
            case DOWNLOAD_INIT_EXPEDITED_2_BYTE:
                sdo_response->length = 2;
                break;
            case UPLOAD_RESPONSE_EXPEDIDED_1_BYTE:
            case DOWNLOAD_INIT_EXPEDITED_1_BYTE:
                sdo_response->length = 1;
                break;
            default:
            case ABORT_TRANSFER: /* Error. */
                abort_code = (abort_code & 0xffffff00) | msg_in.data[7];
                abort_code = (abort_code & 0xffff00ff) | ((Uint32)msg_in.data[6] << 8);
                abort_code = (abort_code & 0xff00ffff) | ((Uint32)msg_in.data[5] << 16);
                abort_code = (abort_code & 0x00ffffff) | ((Uint32)msg_in.data[4] << 24);
                abort_code = SDL_SwapBE32(abort_code);

                SDL_snprintf(reason, 300, "0x%08x: %s", abort_code, lookup_abort_code((abort_code)));
                print_error(reason, sdo_type, node_id, index, sub_index, comment, disp_mode);
                return ABORT_TRANSFER;
        }

        if ((SDO_READ == sdo_type) && (SDL_TRUE == is_normal_sdo))
        {
            Uint8 response_index = 0;
            Uint8 cmd            = UPLOAD_SEGMENT_REQUEST_1;
            Uint8 data_length    = sdo_response->length;
            Uint8 remainder      = data_length % 7u;
            Uint8 expected_msgs  = (data_length / 7u) + (remainder ? 1 : 0);
            
            msg_out.id      = 0x600 + node_id;
            msg_out.length  = 8;
            msg_out.data[0] = cmd;
            
            can_status = can_write(&msg_out, NO_OUTPUT, NULL);
            if (0 != can_status)
            {
                /* Nothing to do here. */
            }
            
            for (n = 0; n < expected_msgs; n += 1)
            {
                timeout_time      = 0;
                time_a            = SDL_GetTicks64();
                response_received = SDL_FALSE;
            
                while ((SDL_FALSE == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
                {
                    Uint64 time_b;
                    Uint64 delta_time;
            
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

                            can_status = can_write(&msg_out, NO_OUTPUT, NULL);
                            if (0 != can_status)
                            {
                                /* Nothing to do here. */
                            }
                        }

                        for (can_msg_index = 1; can_msg_index <= 7; can_msg_index += 1)
                        {
                            char printable_char;
                            if (response_index >= data_length)
                            {
                                break;
                            }
                            else if (SDL_isprint(msg_in.data[can_msg_index]))
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

                        response_received = SDL_TRUE;
                        continue;
                    }

                    time_b = time_a;
                    time_a = SDL_GetTicks64();

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
                    SDL_snprintf(reason, 300, "SDO timeout: CAN-dongle present?");
                    print_error(reason, sdo_type, node_id, index, sub_index, comment, disp_mode);
                    return ABORT_TRANSFER;
                }
            }
         }
        else if (NORMAL_SDO_WRITE == sdo_type)
        {
            Uint8 cmd        = UPLOAD_SEGMENT_CONTINUE_1;
            int   data_index = 0;

            msg_out.id      = 0x600 + node_id;
            msg_out.length  = 8;
            msg_out.data[0] = cmd;

            while (msg_out.data[0] != (cmd | 0x0b))
            {
                int i;

                msg_out.data[0] = cmd;

                for (i = 1; i <= 7; i += 1)
                {
                    char* data_str  = (char*)data;
                    if ('\0' == data_str[data_index])
                    {
                        msg_out.data[0] = cmd | 0x0b; /* Data sent; mark last segment. */
                        msg_out.data[i] = '\n';
                    }
                    else if ((cmd | 0x0b) == msg_out.data[0])
                    {
                        msg_out.data[i] = 0x00; /* Fill remaining bytes with 0x00. */
                    }
                    else
                    {
                        msg_out.data[i]  = data_str[data_index];
                    }
                    data_index += 1;
                }

                can_status = can_write(&msg_out, NO_OUTPUT, NULL);
                if (0 != can_status)
                {
                    /* Nothing to do here. */
                }

                if ((cmd | 0x0b) == msg_out.data[0])
                {
                    break;
                }

                while (timeout_time < SDO_TIMEOUT_IN_MS)
                {
                    Uint64 time_b;
                    Uint64 delta_time;

                    can_read(&msg_in);
                    if ((0x580 + node_id) == msg_in.id)
                    {
                        int can_msg_index;
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
                        break;
                    }
                }

                if (timeout_time >= SDO_TIMEOUT_IN_MS)
                {
                    SDL_snprintf(reason, 300, "SDO timeout: CAN-dongle present?");
                    print_error(reason, sdo_type, node_id, index, sub_index, comment, disp_mode);
                    return ABORT_TRANSFER;
                }
            }
        }
        else /* Write expedided SDO. */
        {
            for (n = 0; n < sdo_response->length; n += 1)
            {
                sdo_response->data[4 + n] = msg_in.data[4 + n];
            }
        }
    }

    if (SDO_READ == sdo_type)
    {
        print_read_result(node_id, index, sub_index, sdo_response, disp_mode, is_normal_sdo, comment);
    }
    else
    {
        print_write_result(sdo_type, node_id, index, sub_index, sdo_response->length, data, disp_mode, comment);
    }

    return command_code;
}

static const char* lookup_abort_code(Uint32 abort_code)
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

static void print_error(const char* reason, sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, const char* comment, disp_mode_t disp_mode)
{
    switch (disp_mode)
    {
        case NORMAL_OUTPUT:
        {
            const  char* description = dict_lookup(index, sub_index);

            if (NULL != description)
            {
                c_log(LOG_INFO, "%s", description);
            }

            switch (sdo_type)
            {
                case SDO_READ:
                    c_log(LOG_ERROR, "Index %x, Sub-index %x: 0 byte(s) read error: %s", index, sub_index, reason);
                    break;
                case EXPEDITED_SDO_WRITE:
                case NORMAL_SDO_WRITE:
                    c_log(LOG_ERROR, "Index %x, Sub-index %x: 0 byte(s) write error: %s", index, sub_index, reason);
                    break;
                default:
                    break;
            }
            break;
        }

        case SCRIPT_OUTPUT:
        {
            int     i;
            char    buffer[34] = { 0 };
            color_t color = DARK_YELLOW;

            switch (sdo_type)
            {
                case SDO_READ:
                    c_printf(color, "Read ");
                    c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -       ", node_id, index, sub_index);
                    break;
                case EXPEDITED_SDO_WRITE:
                case NORMAL_SDO_WRITE:
                    color = LIGHT_BLUE;
                    c_printf(color, "Write");
                    c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -        ", node_id, index, sub_index);
                    break;
                default:
                    break;
            }

            c_printf(LIGHT_RED, "FAIL    ");

            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            SDL_strlcpy(buffer, comment, 33);
            for (i = SDL_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            c_printf(DARK_MAGENTA, "%s ", buffer);
            c_printf(DEFAULT_COLOR, "%s\n", reason);
            break;
        }
        default:
        case NO_OUTPUT:
            break;
    }
}

void print_read_result(Uint8 node_id, Uint16 index, Uint8 sub_index, can_message_t* sdo_response, disp_mode_t disp_mode, SDL_bool is_normal_sdo, const char* comment)
{
    Uint32 u32_value = (Uint32)sdo_response->data[4];

    switch (disp_mode)
    {
        case NORMAL_OUTPUT:
        {
            const char* description = dict_lookup(index, sub_index);

            if (NULL != description)
            {
                c_log(LOG_INFO, "%s", description);
            }

            if (SDL_TRUE == is_normal_sdo)
            {
                sdo_response->data[CAN_MAX_DATA_LENGTH] = '\0';
                c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) read: %s",
                    index,
                    sub_index,
                    sdo_response->length,
                    (char*)sdo_response->data);
            }
            else
            {
                c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) read: %u (0x%x)",
                    index,
                    sub_index,
                    sdo_response->length,
                    u32_value,
                    u32_value);
            }
            break;
        }
        case SCRIPT_OUTPUT:
        {
            int  i;
            char buffer[34] = { 0 };

            sdo_response->data[CAN_MAX_DATA_LENGTH] = '\0';

            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            SDL_strlcpy(buffer, comment, 33);
            for (i = SDL_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            c_printf(DARK_YELLOW, "Read     ");
            c_printf(DEFAULT_COLOR, "0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, sdo_response->length);
            c_printf(LIGHT_GREEN, "SUCC    ");
            c_printf(DARK_MAGENTA, "%s ", buffer);

            if (SDL_FALSE == is_normal_sdo)
            {
                switch (sdo_response->length)
                {
                    case 4:
                        c_printf(DEFAULT_COLOR, "0x%08X %u (U32)", u32_value, u32_value);
                        break;
                    case 3:
                        c_printf(DEFAULT_COLOR, "0x%06X %u (U24)", u32_value, u32_value);
                        break;
                    case 2:
                        c_printf(DEFAULT_COLOR, "0x%04X %u (U16)", u32_value, u32_value);
                        break;
                    case 1:
                        c_printf(DEFAULT_COLOR, "0x%02X %u (U8)", u32_value, u32_value);
                        break;
                }
            }
            else
            {
                c_printf(DEFAULT_COLOR, "%s", (char*)sdo_response->data);
            }
            puts("");
            break;
        }
        default:
        case NO_OUTPUT:
            break;
    }
}

void print_write_result(sdo_type_t sdo_type, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void* data, disp_mode_t disp_mode, const char* comment)
{
    Uint32  u32_value    = 0;
    Uint32* u32_data_ptr = (Uint32*)data;
    char*   data_str     = (char*)data;

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
        case NORMAL_OUTPUT:
        {
            const char* description = dict_lookup(index, sub_index);
        
            if (NULL != description)
            {
                c_log(LOG_INFO, "%s", description);
            }

            if (EXPEDITED_SDO_WRITE == sdo_type)
            {
                c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) written: %u (0x%x)",
                    index,
                    sub_index,
                    length,
                    u32_value,
                    u32_value);
            }
            else if (NORMAL_SDO_WRITE)
            {
                data_str[CAN_MAX_DATA_LENGTH] = '\0';
    
                c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) written: %s",
                    index,
                    sub_index,
                    length,
                    data_str);
            }
            else
            {
                return;
            }
            break;
        }
        case SCRIPT_OUTPUT:
        {
            int  i;
            char buffer[34] = { 0 }; 
            
            if (NULL == comment)
            {
                comment = dict_lookup(index, sub_index);
            }

            if (NULL == comment)
            {
                comment = "-";
            }

            SDL_strlcpy(buffer, comment, 33);
            for (i = SDL_strlen(buffer); i < 33; ++i)
            {
                buffer[i] = ' ';
            }

            c_printf(DARK_BLUE, "Write    ");
            c_printf(DEFAULT_COLOR, "0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, length);
            c_printf(LIGHT_GREEN, "SUCC    ");
            c_printf(DARK_MAGENTA, "%s ", buffer);

            if (EXPEDITED_SDO_WRITE == sdo_type)
            {
                switch (length)
                {
                    case 4:
                        c_printf(DEFAULT_COLOR, "0x%08X %u (U32)", u32_value, u32_value);
                        break;
                    case 3:
                        c_printf(DEFAULT_COLOR, "0x%06X %u (U24)", u32_value, u32_value);
                        break;
                    case 2:
                        c_printf(DEFAULT_COLOR, "0x%04X %u (U16)", u32_value, u32_value);
                        break;
                    case 1:
                        c_printf(DEFAULT_COLOR, "0x%02X %u (U8)", u32_value, u32_value);
                        break;
                }
            }
            else
            {
                data_str[SDL_strlen(data_str) - 1] = '\0'; /* Strip possible newline. */
                c_printf(DEFAULT_COLOR, "%s", data_str);
            }
            puts("");

            break;
        }
        case NO_OUTPUT:
            break;
    }
}
