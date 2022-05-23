/** @file sdo_client.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifdef _WIN32
#include <windows.h>
#endif
#include "PCANBasic.h"

#include "SDL.h"
#include "nuklear.h"
#include "can.h"
#include "core.h"
#include "sdo_client.h"

#define SDO_TIMEOUT_IN_MS 100

Uint32 read_sdo(can_message_t* sdo_response, Uint8 node_id, Uint16 index, Uint8 sub_index)
{
    TPCANMsg can_message       = { 0 };
    Uint32   can_status        = PCAN_ERROR_OK;
    SDL_bool response_received = SDL_FALSE;
    Uint64   timeout_time      = 0;
    Uint64   time_a;

    if (node_id > 0x7f)
    {
        node_id = 0x00 + (node_id % 0x7f);
    }

    can_message.ID      = 0x600 + node_id;
    can_message.MSGTYPE = PCAN_MESSAGE_STANDARD;
    can_message.LEN     = 8;

    can_message.DATA[0] = READ_DICT_OBJECT;
    can_message.DATA[1] = (Uint8)(index  & 0x00ff);
    can_message.DATA[2] = (Uint8)((index & 0xff00) >> 8);
    can_message.DATA[3] = sub_index;

    can_status = CAN_Write(PCAN_USBBUS1, &can_message);
    if (PCAN_ERROR_OK != can_status)
    {
        char err_message[100] = { 0 };
        CAN_GetErrorText(can_status, 0x09, err_message);
        SDL_LogWarn(0, "Could not send read SDO packet: %s", err_message);
    }

    time_a = SDL_GetTicks64();
    while ((SDL_FALSE == response_received) && (timeout_time < SDO_TIMEOUT_IN_MS))
    {
        Uint64 time_b;
        Uint64 delta_time;

        if ((0x580 + node_id) == can_message.ID)
        {
            if ((index  & 0x00ff) == can_message.DATA[1])
            {
                if (((index & 0xff00) >> 8) == can_message.DATA[2])
                {
                    response_received = SDL_TRUE;
                    continue;
                }
            }
        }

        can_status = CAN_Read(PCAN_USBBUS1, &can_message, NULL);
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
    if (PCAN_ERROR_OK != can_status)
    {
        char err_message[100] = { 0 };
        CAN_GetErrorText(can_status, 0x09, err_message);
        SDL_LogWarn(0, "Could not receive SDO response: %s", err_message);
        sdo_response = NULL;
    }
    else if (timeout_time >= SDO_TIMEOUT_IN_MS)
    {
        SDL_LogWarn(0, "SDO timeout: USB dongle present?");
        sdo_response = NULL;
    }
    else
    {
        Uint32 abort_code = 0;
        int    data_index;
        switch (can_message.DATA[0])
        {
            case READ_DICT_4_BYTE_SEND:
                sdo_response->length = 4;
                break;
            case READ_DICT_3_BYTE_SEND:
                sdo_response->length = 3;
                break;
            case READ_DICT_2_BYTE_SEND:
                sdo_response->length = 2;
                break;
            case READ_DICT_1_BYTE_SEND:
                sdo_response->length = 1;
                break;
            case READ_ABORT:
                abort_code = (abort_code & 0xffffff00) | can_message.DATA[7];
                abort_code = (abort_code & 0xffff00ff) | ((Uint32)can_message.DATA[6] << 8);
                abort_code = (abort_code & 0xff00ffff) | ((Uint32)can_message.DATA[5] << 16);
                abort_code = (abort_code & 0x00ffffff) | ((Uint32)can_message.DATA[4] << 24);
                abort_code = SDL_SwapBE32(abort_code);

                switch(abort_code)
                {
                    case ABORT_TOGGLE_BIT_NOT_ALTERED:
                        SDL_LogWarn(0, "Toggle bit not altered");
                        break;
                    case ABORT_SDO_PROTOCOL_TIMED_OUT:
                        SDL_LogWarn(0, "SDO protocol timed out");
                        break;
                    case ABORT_CMD_SPECIFIER_INVALID_UNKNOWN:
                        SDL_LogWarn(0, "Client/server command specifier not valid or unknown");
                        break;
                    case ABORT_INVALID_BLOCK_SIZE:
                        SDL_LogWarn(0, "Invalid block size");
                        break;
                    case ABORT_INVALID_SEQUENCE_NUMBER:
                        SDL_LogWarn(0, "Invalid sequence number");
                        break;
                    case ABORT_CRC_ERROR:
                        SDL_LogWarn(0, "CRC error");
                        break;
                    case ABORT_OUT_OF_MEMORY:
                        SDL_LogWarn(0, "Out of memory");
                        break;
                    case ABORT_UNSUPPORTED_ACCESS:
                        SDL_LogWarn(0, "Unsupported access to an object");
                        break;
                    case ABORT_ATTEMPT_TO_READ_WRITE_ONLY:
                        SDL_LogWarn(0, "Attempt to read a write only object");
                        break;
                    case ABORT_ATTEMPT_TO_WRITE_READ_ONLY:
                        SDL_LogWarn(0, "Attempt to write a read only object");
                        break;
                    case ABORT_OBJECT_DOES_NOT_EXIST:
                        SDL_LogWarn(0, "Object does not exist in the object dictionary");
                        break;
                    case ABORT_OBJECT_CANNOT_BE_MAPPED:
                        SDL_LogWarn(0, "Object cannot be mapped to the PDO");
                        break;
                    case ABORT_WOULD_EXCEED_PDO_LENGTH:
                        SDL_LogWarn(0, "Number, length of the object would exceed PDO length");
                        break;
                    case ABORT_GENERAL_INCOMPATIBILITY_REASON:
                        SDL_LogWarn(0, "General parameter incompatibility reason");
                        break;
                    case ABORT_GENERAL_INTERNAL_INCOMPATIBILITY:
                        SDL_LogWarn(0, "General internal incompatibility in the device");
                        break;
                    case ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR:
                        SDL_LogWarn(0, "Access failed due to an hardware error");
                        break;
                    case ABORT_DATA_TYPE_DOES_NOT_MATCH:
                        SDL_LogWarn(0, "Data type does not match, length does not match");
                        break;
                    case ABORT_DATA_TYPE_LENGTH_TOO_HIGH:
                        SDL_LogWarn(0, "Data type does not match, length too high");
                        break;
                    case ABORT_DATA_TYPE_LENGTH_TOO_LOW:
                        SDL_LogWarn(0, "Data type does not match, length too low");
                        break;
                    case ABORT_SUB_INDEX_DOES_NOT_EXIST:
                        SDL_LogWarn(0, "Sub-index does not exist");
                        break;
                    case ABORT_INVALID_VALUE_FOR_PARAMETER:
                        SDL_LogWarn(0, "Invalid value for parameter");
                        break;
                    case ABORT_VALUE_FOR_PARAMETER_TOO_HIGH:
                        SDL_LogWarn(0, "Value for parameter written too high");
                        break;
                    case ABORT_VALUE_FOR_PARAMETER_TOO_LOW:
                        SDL_LogWarn(0, "Value for parameter written too low");
                        break;
                    case ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE:
                        SDL_LogWarn(0, "Maximum value is less than minimum value");
                        break;
                    case ABORT_RESOURCE_NOT_AVAILABLE:
                        SDL_LogWarn(0, "Resource not available: SDO connection");
                        break;
                    case ABORT_GENERAL_ERROR:
                        SDL_LogWarn(0, "General error");
                        break;
                    case ABORT_DATA_CANNOT_BE_TRANSFERRED:
                        SDL_LogWarn(0, "Data cannot be transferred");
                        break;
                    case ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL:
                        SDL_LogWarn(0, "Data cannot be transferred or stored to the application because of local control");
                        break;
                    case ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE:
                        SDL_LogWarn(0, "Data cannot be transferred because of the present device state");
                        break;
                    case ABORT_NO_OBJECT_DICTIONARY_PRESENT:
                        SDL_LogWarn(0, "Object dictionary dynamic generation fails or no object dictionary present");
                        break;
                    case ABORT_NO_DATA_AVAILABLE:
                        SDL_LogWarn(0, "No data available");
                        break;
                }
                return can_status;
        }

        for (data_index = 0; data_index < sdo_response->length; data_index += 1)
        {
            sdo_response->data[data_index] = can_message.DATA[4 + data_index];
        }

        SDL_Log("%u byte(s) received: %u (0x%x)",
                sdo_response->length,
                (Uint32)*sdo_response->data,
                (Uint32)*sdo_response->data);
    }

    return can_status;
}
