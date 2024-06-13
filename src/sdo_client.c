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
#include "can.h"
#include "core.h"
#include "dict.h" 
#include "printf.h"
#include "sdo_client.h"

#define SDO_TIMEOUT_IN_MS 100

static Uint32      sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, output_mode_t output_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void *data, const char* comment);
static const char* lookup_abort_code(Uint32 abort_code);
static void        print_abort_code_error(Uint32 abort_code, output_mode_t output_mode);

Uint32 sdo_read(can_message_t* sdo_response,output_mode_t output_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, const char* comment)
{
    return sdo_send(
        SDO_READ,
        sdo_response,
        output_mode,
        node_id,
        index,
        sub_index,
        0, 0,
        comment);
}

Uint32 sdo_write(can_message_t* sdo_response, output_mode_t output_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void *data, const char* comment)
{
    return sdo_send(
        EXPEDITED_SDO_WRITE,
        sdo_response,
        output_mode,
        node_id,
        index,
        sub_index,
        length,
        data,
        comment);
}

int lua_sdo_read(lua_State* L)
{
    can_message_t sdo_response     = { 0 };
    output_mode_t output_mode  = NO_OUTPUT;
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
        output_mode = SCRIPT_OUTPUT;
    }

    status = sdo_read(
        &sdo_response,
        output_mode,
        (Uint8)node_id,
        (Uint16)index,
        (Uint16)sub_index,
        comment);

    switch (status)
    {
        case READ_DICT_OBJECT:
        case READ_DICT_SIZE_INDICATED:
            lua_pushstring(L, (const char*)sdo_response.data);
            break;
        case SDO_ABORT:
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
    output_mode_t output_mode  = NO_OUTPUT;
    int           node_id      = luaL_checkinteger(L, 1);
    int           index        = luaL_checkinteger(L, 2);
    int           sub_index    = luaL_checkinteger(L, 3);
    int           length       = luaL_checkinteger(L, 4);
    int           data         = luaL_checkinteger(L, 5);
    int           status;
    SDL_bool      show_output = lua_toboolean(L, 6);
    const char*   comment     = lua_tostring(L, 7);

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    if (SDL_TRUE == show_output)
    {
        output_mode = SCRIPT_OUTPUT;
    }

    status = sdo_write(
        &sdo_response,
        output_mode,
        (Uint8)node_id,
        (Uint16)index,
        (Uint8)sub_index,
        (Uint8)length,
        (void*)&data,
        comment);

    switch (status)
    {
        case SDO_ABORT:
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
}

static Uint32 sdo_send(sdo_type_t sdo_type, can_message_t* sdo_response, output_mode_t output_mode, Uint8 node_id, Uint16 index, Uint8 sub_index, Uint8 length, void* data, const char* comment)
{
    can_message_t msg_in            = { 0 };
    can_message_t msg_out           = { 0 };
    Uint32        can_status        = 0;
    Uint8         command_code      = 0x00;
    SDL_bool      response_received = SDL_FALSE;
    Uint64        timeout_time      = 0;
    Uint64        time_a;
    Uint32*       u32_data_ptr      = (Uint32*)data;
    Uint32        u32_value = 0;

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
            msg_out.length = 8;
            msg_out.data[0] = READ_DICT_OBJECT;
            break;
        case EXPEDITED_SDO_WRITE:
        {
            if (NULL != u32_data_ptr)
            {
                u32_value = *u32_data_ptr;
            }

            msg_out.length = 4 + length;
            msg_out.data[4] = (Uint8)(u32_value & 0x000000ff);
            msg_out.data[5] = (Uint8)((u32_value & 0x0000ff00) >> 8);
            msg_out.data[6] = (Uint8)((u32_value & 0x00ff0000) >> 16);
            msg_out.data[7] = (Uint8)((u32_value & 0xff000000) >> 24);

            switch (length)
            {
                case 1:
                    msg_out.data[0] = WRITE_DICT_1_BYTE_SENT;
                    break;
                case 2:
                    msg_out.data[0] = WRITE_DICT_2_BYTE_SENT;
                    break;
                case 3:
                    msg_out.data[0] = WRITE_DICT_3_BYTE_SENT;
                    break;
                case 4:
                default:
                    msg_out.data[0] = WRITE_DICT_4_BYTE_SENT;
                    break;
            }
            break;
        }
    }

    can_status = can_write(&msg_out);
    if (0 != can_status)
    {
        can_print_error_message(NULL, can_status, SDL_TRUE);
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
        if (NORMAL_OUTPUT == output_mode)
        {
            c_log(LOG_WARNING, "SDO timeout: CAN-dongle present?");
        }
        else if (SCRIPT_OUTPUT == output_mode)
        {
            int     i;
            char    buffer[34] = { 0 };
            color_t color      = DARK_YELLOW;

            switch (msg_out.data[0])
            {
                case READ_DICT_OBJECT:
                    c_printf(color, "Read ");
                    c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -       ", node_id, index, sub_index);
                    break;
                case WRITE_DICT_OBJECT:
                case WRITE_DICT_1_BYTE_SENT:
                case WRITE_DICT_2_BYTE_SENT:
                case WRITE_DICT_3_BYTE_SENT:
                case WRITE_DICT_4_BYTE_SENT:
                    color = LIGHT_BLUE;
                    c_printf(color, "Write");
                    c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, length);
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
            c_printf(DEFAULT_COLOR, "%s SDO timeout\n", buffer);
        }

        return SDO_ABORT;
    }
    else
    {
        Uint32 abort_code = 0;
        int    n;

        switch (command_code)
        {
            case WRITE_DICT_OBJECT:
                sdo_response->length = msg_out.length;
                break;
            case READ_DICT_NO_SIZE: /* Normal SDO. */
            case READ_DICT_SIZE_INDICATED:
                sdo_response->length = msg_in.data[4];
                break;
            case READ_DICT_4_BYTE_SENT: /* Expedided SDO. */
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
            default:
            case SDO_ABORT: /* Error. */
                abort_code = (abort_code & 0xffffff00) | msg_in.data[7];
                abort_code = (abort_code & 0xffff00ff) | ((Uint32)msg_in.data[6] << 8);
                abort_code = (abort_code & 0xff00ffff) | ((Uint32)msg_in.data[5] << 16);
                abort_code = (abort_code & 0x00ffffff) | ((Uint32)msg_in.data[4] << 24);
                abort_code = SDL_SwapBE32(abort_code);

                if (NORMAL_OUTPUT == output_mode)
                {
                    print_abort_code_error(abort_code, output_mode);
                }
                else if (SCRIPT_OUTPUT)
                {
                    int     i;
                    char    buffer[34] = { 0 };
                    color_t color = DARK_YELLOW;

                    switch (msg_out.data[0])
                    {
                        case READ_DICT_OBJECT:
                            c_printf(color, "Read ");
                            c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      -       ", node_id, index, sub_index);
                            break;
                        case WRITE_DICT_OBJECT:
                        case WRITE_DICT_1_BYTE_SENT:
                        case WRITE_DICT_2_BYTE_SENT:
                        case WRITE_DICT_3_BYTE_SENT:
                        case WRITE_DICT_4_BYTE_SENT:
                            color = LIGHT_BLUE;
                            c_printf(color, "Write");
                            c_printf(DEFAULT_COLOR, "    0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, length);
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
                    c_printf(DEFAULT_COLOR, "0x%08X: %s\n", abort_code, lookup_abort_code(abort_code));
                }

                return SDO_ABORT;
        }

        switch (command_code)
        {
            case READ_DICT_NO_SIZE: /* Normal SDO. */
            case READ_DICT_SIZE_INDICATED:
            {
                Uint8         sdo_response_index = 0;
                Uint8         cmd                = 0x60;
                Uint8         remainder          = sdo_response->length % 7u;
                Uint8         expected_msgs      = (sdo_response->length / 7u) + (remainder ? 1 : 0);

                msg_out.id      = 0x600 + node_id;
                msg_out.length  = 8;
                msg_out.data[0] = cmd;

                can_status = can_write(&msg_out);
                if (0 != can_status)
                {
                    can_print_error_message(NULL, can_status, SDL_TRUE);
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
                            msg_out.data[0]  = cmd;
                            int can_msg_index = 0;

                            if (0 == (msg_in.data[0] % 2))
                            {
                                if (0x60 == cmd)
                                {
                                    cmd = 0x70;
                                }
                                else
                                {
                                    cmd = 0x60;
                                }

                                msg_out.data[0] = cmd;

                                can_status = can_write(&msg_out);
                                if (0 != can_status)
                                {
                                    can_print_error_message(NULL, can_status, SDL_TRUE);
                                }
                            }

                            for (can_msg_index = 1; can_msg_index < 8; can_msg_index += 1)
                            {
                                sdo_response->data[sdo_response_index] = msg_in.data[can_msg_index];
                                sdo_response_index += 1;
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
                        if (NORMAL_OUTPUT == output_mode)
                        {
                            c_log(LOG_WARNING, "SDO timeout: CAN-dongle present?");
                        }
                        return SDO_ABORT;
                    }
                }
                break;
            }
            default: /* Expedided SDO. */
                for (n = 0; n < sdo_response->length;n += 1)
                {
                    sdo_response->data[4 + n] = msg_in.data[4 + n];
                }
                break;
        }
    }

    switch (output_mode)
    {
        case NORMAL_OUTPUT:
        {
            const  char*  description = dict_lookup(index, sub_index);
            const  char*  str_read    = "read";
            const  char*  str_written = "written";
            const  char** str_action  = &str_read;
            Uint32        output_data = 0;

            switch (command_code)
            {
                case READ_DICT_NO_SIZE:
                case READ_DICT_SIZE_INDICATED:
                    str_action = &str_read;
                    break;
                case READ_DICT_4_BYTE_SENT:
                case READ_DICT_3_BYTE_SENT:
                case READ_DICT_2_BYTE_SENT:
                case READ_DICT_1_BYTE_SENT:
                    output_data = (Uint32)sdo_response->data[4];
                    str_action = &str_read;
                    break;
                case WRITE_DICT_OBJECT:
                case WRITE_DICT_4_BYTE_SENT:
                case WRITE_DICT_3_BYTE_SENT:
                case WRITE_DICT_2_BYTE_SENT:
                case WRITE_DICT_1_BYTE_SENT:
                    output_data = u32_value;
                    str_action = &str_written;
                    break;
            }

            if (NULL != description)
            {
                c_log(LOG_INFO, "%s", description);
            }

            switch (command_code)
            {
                case READ_DICT_NO_SIZE:
                case READ_DICT_SIZE_INDICATED:
                    sdo_response->data[CAN_MAX_DATA_LENGTH] = '\0';
                    c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) %s: %s",
                        index,
                        sub_index,
                        sdo_response->length,
                        *str_action,
                        (char*)sdo_response->data);
                    break;
                default:
                    c_log(LOG_SUCCESS, "Index %x, Sub-index %x: %u byte(s) %s: %u (0x%x)",
                        index,
                        sub_index,
                        sdo_response->length,
                        *str_action,
                        output_data,
                        output_data);
                    break;
            }
            break;
        }
        case SCRIPT_OUTPUT:
        {
            int           i;
            char          buffer[34]  = { 0 };
            const  char*  str_read    = "Read    ";
            const  char*  str_written = "Write   ";
            const  char** str_action  = &str_read;
            Uint32        output_data = 0;
            color_t       color       = DARK_YELLOW;

            switch (command_code)
            {
                case READ_DICT_NO_SIZE:
                case READ_DICT_SIZE_INDICATED:
                    str_action = &str_read;
                    break;
                case READ_DICT_4_BYTE_SENT:
                case READ_DICT_3_BYTE_SENT:
                case READ_DICT_2_BYTE_SENT:
                case READ_DICT_1_BYTE_SENT:
                    output_data = (Uint32)sdo_response->data[4];
                    str_action = &str_read;
                    break;
                case WRITE_DICT_OBJECT:
                case WRITE_DICT_4_BYTE_SENT:
                case WRITE_DICT_3_BYTE_SENT:
                case WRITE_DICT_2_BYTE_SENT:
                case WRITE_DICT_1_BYTE_SENT:
                    output_data = u32_value;
                    str_action  = &str_written;
                    color       = LIGHT_BLUE;
                    break;
            }

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

            switch (command_code)
            {
                case READ_DICT_NO_SIZE:
                case READ_DICT_SIZE_INDICATED:
                    sdo_response->data[CAN_MAX_DATA_LENGTH] = '\0';
                    break;
                default:
                    break;
            }

            c_printf(color, "%s", *str_action);
            c_printf(DEFAULT_COLOR, " 0x%02X    0x%04X  0x%02X      %03u     ", node_id, index, sub_index, length);
            c_printf(LIGHT_GREEN, "SUCC    ");
            c_printf(DARK_MAGENTA, "%s ", buffer);

            switch (command_code)
            {
                case READ_DICT_NO_SIZE:
                case READ_DICT_SIZE_INDICATED:
                    c_printf(DEFAULT_COLOR, "%s", (char*)sdo_response->data);
                    break;
                case READ_DICT_4_BYTE_SENT:
                case WRITE_DICT_4_BYTE_SENT:
                    c_printf(DEFAULT_COLOR, "0x%08X %u (U32)", output_data, output_data);
                    break;
                case READ_DICT_3_BYTE_SENT:
                case WRITE_DICT_3_BYTE_SENT:
                    c_printf(DEFAULT_COLOR, "0x%06X %u (U24)", output_data, output_data);
                    break;
                case READ_DICT_2_BYTE_SENT:
                case WRITE_DICT_2_BYTE_SENT:
                    c_printf(DEFAULT_COLOR, "0x%04X %u (U16)", output_data, output_data);
                    break;
                case READ_DICT_1_BYTE_SENT:
                case WRITE_DICT_1_BYTE_SENT:
                    c_printf(DEFAULT_COLOR, "0x%02X %u (U8)", output_data, output_data);
                    break;
                default:
                    break;
            }

            puts("");
            break;
        }
        default:
        case NO_OUTPUT:
            break;
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

static void print_abort_code_error(Uint32 abort_code, output_mode_t output_mode)
{
    if (NORMAL_OUTPUT != output_mode)
    {
        return;
    }

    c_log(LOG_WARNING, "%s", lookup_abort_code(abort_code));
}
