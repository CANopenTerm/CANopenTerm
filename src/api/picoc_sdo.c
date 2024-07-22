/** @file picoc_sdo.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "can.h"
#include "core.h"
#include "dict.h"
#include "interpreter.h"
#include "os.h"
#include "picoc_sdo.h"
#include "sdo.h"

static can_message_t sdo_response  = { 0 };
static char          str_buffer[5] = { 0 };

extern bool_t is_printable_string(const char *str, size_t length);

static const char defs[] = "                               \
typedef enum {                                             \
    ABORT_TOGGLE_BIT_NOT_ALTERED             = 0x05030000, \
    ABORT_SDO_PROTOCOL_TIMED_OUT             = 0x05040000, \
    ABORT_CMD_SPECIFIER_INVALID_UNKNOWN      = 0x05040001, \
    ABORT_INVALID_BLOCK_SIZE                 = 0x05040002, \
    ABORT_INVALID_SEQUENCE_NUMBER            = 0x05040003, \
    ABORT_CRC_ERROR                          = 0x05040004, \
    ABORT_OUT_OF_MEMORY                      = 0x05040005, \
    ABORT_UNSUPPORTED_ACCESS                 = 0x06010000, \
    ABORT_ATTEMPT_TO_READ_WRITE_ONLY         = 0x06010001, \
    ABORT_ATTEMPT_TO_WRITE_READ_ONLY         = 0x06010002, \
    ABORT_OBJECT_DOES_NOT_EXIST              = 0x06020000, \
    ABORT_OBJECT_CANNOT_BE_MAPPED            = 0x06040041, \
    ABORT_WOULD_EXCEED_PDO_LENGTH            = 0x06040042, \
    ABORT_GENERAL_INCOMPATIBILITY_REASON     = 0x06040043, \
    ABORT_GENERAL_INTERNAL_INCOMPATIBILITY   = 0x06040047, \
    ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR   = 0x06060000, \
    ABORT_DATA_TYPE_DOES_NOT_MATCH           = 0x06070010, \
    ABORT_DATA_TYPE_LENGTH_TOO_HIGH          = 0x06070012, \
    ABORT_DATA_TYPE_LENGTH_TOO_LOW           = 0x06070013, \
    ABORT_SUB_INDEX_DOES_NOT_EXIST           = 0x06090011, \
    ABORT_INVALID_VALUE_FOR_PARAMETER        = 0x06090030, \
    ABORT_VALUE_FOR_PARAMETER_TOO_HIGH       = 0x06090031, \
    ABORT_VALUE_FOR_PARAMETER_TOO_LOW        = 0x06090032, \
    ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE      = 0x06090036, \
    ABORT_RESOURCE_NOT_AVAILABLE             = 0x060a0023, \
    ABORT_GENERAL_ERROR                      = 0x08000000, \
    ABORT_DATA_CANNOT_BE_TRANSFERRED         = 0x08000020, \
    ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL = 0x08000021, \
    ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE  = 0x08000022, \
    ABORT_NO_OBJECT_DICTIONARY_PRESENT       = 0x08000023, \
    ABORT_NO_DATA_AVAILABLE                  = 0x08000024  \
} sdo_abort_code_t;";

static void c_sdo_lookup_abort_code(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_sdo_read(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_sdo_write(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_sdo_write_file(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_sdo_write_string(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void c_dict_lookup(struct ParseState *parser, struct Value *return_value, struct Value **param, int args);
static void setup(Picoc* P);

struct LibraryFunction picoc_sdo_functions[] =
{
    { c_sdo_lookup_abort_code, "char* sdo_lookup_abort_code(sdo_abort_code_t abort_code);" },
    { c_sdo_read,              "char* sdo_read(unsigned int* result, int node_id, int index, int sub_index, int show_output, char* comment);"},
    { c_sdo_write,             "int sdo_write(int node_id, int index, int sub_index, int length, char* data, int show_output, char* comment);"},
    { c_sdo_write_file,        "int sdo_write_file(int node_id, int index, int sub_index, char* filename);"},
    { c_sdo_write_string,      "int sdo_write_string(int node_id, int index, int sub_index, char* data);"},
    { c_dict_lookup,           "char* dict_lookup(int index, int sub_index);"},
    { NULL, NULL }
};

void picoc_sdo_init(core_t* core)
{
    IncludeRegister(&core->P, "sdo.h", &setup, &picoc_sdo_functions[0], defs);
}

static void c_sdo_lookup_abort_code(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    sdo_abort_code_t abort_code = (sdo_abort_code_t)param[0]->Val->UnsignedInteger;

    return_value->Val->Pointer = (void *)sdo_lookup_abort_code(abort_code);
}

static void c_sdo_read(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    disp_mode_t   disp_mode   = SILENT;
    sdo_state_t   sdo_state;
    int           node_id     = param[1]->Val->Integer;
    int           index       = param[2]->Val->Integer;
    int           sub_index   = param[3]->Val->Integer;
    int           show_output = param[4]->Val->Integer;
    const char*   comment     = (const char *)param[5]->Val->Pointer;

    limit_node_id((uint8 *)&node_id);

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    sdo_state = sdo_read(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint16)sub_index,
        comment);

    switch (sdo_state)
    {
        case IS_READ_SEGMENTED:
            return_value->Val->Pointer = (void *)sdo_response.data;
            break;
        case IS_READ_EXPEDITED:
            os_memcpy(&str_buffer, &sdo_response.data, sizeof(uint32));
            os_memcpy(param[0]->Val->Pointer, &sdo_response.data, sizeof(uint32));

            if (is_printable_string(str_buffer, sizeof(uint32)))
            {
                return_value->Val->Pointer = (void*)str_buffer;
            }
            else
            {
                return_value->Val->Pointer = NULL;
            }
            break;
        default:
        case ABORT_TRANSFER:
            return_value->Val->Pointer = NULL;
            break;
    }
}

static void c_sdo_write(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    disp_mode_t disp_mode   = SILENT;
    sdo_state_t sdo_state;
    int         node_id     = param[0]->Val->Integer;
    int         index       = param[1]->Val->Integer;
    int         sub_index   = param[2]->Val->Integer;
    int         length      = param[3]->Val->Integer;
    bool_t      show_output = param[5]->Val->Integer;
    uint32      data        = 0;
    const char* comment     = (const char*)param[6]->Val->Pointer;

    limit_node_id((uint8 *)&node_id);

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    if (length > sizeof(uint32))
    {
        length = sizeof(uint32);
    }

    os_memcpy(&data, param[4]->Val->Pointer, sizeof(uint32));
    data = os_swap_be_32(data);

    sdo_state = sdo_write(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        (uint32)length,
        (void *)&data,
        comment);

    switch (sdo_state)
    {
        case ABORT_TRANSFER:
            return_value->Val->Integer = 0;
            break;
        default:
            return_value->Val->Integer = 1;
            break;
    }
}

static void c_sdo_write_file(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    disp_mode_t disp_mode = SILENT;
    int         status;
    int         node_id   = param[0]->Val->Integer;
    int         index     = param[1]->Val->Integer;
    int         sub_index = param[2]->Val->Integer;
    const char* filename  = (const char*)param[0]->Val->Pointer;

    if (NULL == filename)
    {
        return_value->Val->Integer = 0;
        return;
    }

    status = sdo_write_block(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        filename,
        NULL);

    switch (status)
    {
        case ABORT_TRANSFER:
            return_value->Val->Integer = 0;
            break;
        default:
            return_value->Val->Integer = 1;
            break;
    }
}

static void c_sdo_write_string(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{

    disp_mode_t disp_mode   = SILENT;
    int         status;
    int         node_id     = param[0]->Val->Integer;
    int         index       = param[1]->Val->Integer;
    int         sub_index   = param[2]->Val->Integer;
    const char* data        = (const char*)param[3]->Val->Pointer;
    uint32      length      = 0;
    bool_t      show_output = param[4]->Val->Integer;
    const char* comment     = (const char*)param[5]->Val->Pointer;

    if (NULL != data)
    {
        length = os_strlen(data);
    }
    else
    {
        return_value->Val->Integer = 0;
        return;
    }

    if (IS_TRUE == show_output)
    {
        disp_mode = SCRIPT_MODE;
    }

    status = sdo_write_segmented(
        &sdo_response,
        disp_mode,
        (uint8)node_id,
        (uint16)index,
        (uint8)sub_index,
        length,
        (void *)data,
        comment);

    switch (status)
    {
        case ABORT_TRANSFER:
            return_value->Val->Integer = 0;
            break;
        default:
            return_value->Val->Integer = 1;
            break;
    }
}

static void c_dict_lookup(struct ParseState *parser, struct Value *return_value, struct Value **param, int args)
{
    int index     = param[0]->Val->Integer;
    int sub_index = param[0]->Val->Integer;

    return_value->Val->Pointer = (void*)dict_lookup(index, sub_index);
}

static void setup(Picoc* P)
{
    (void)P;
}
