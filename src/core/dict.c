/** @file dict.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "os.h"
#include "codb.h"
#include "core.h"
#include "dict.h"
#include "sdo.h"
#include "table.h"

static const emcy_entry_t emcy_table[] =
{
    { 0x0000, "Error Reset or No Error" },
    { 0x1000, "Generic Error" },
    { 0x2000, "Current" },
    { 0x2100, "Current, device input side" },
    { 0x2200, "Current, inside the device" },
    { 0x2300, "Current, device output side" },
    { 0x3000, "Voltage" },
    { 0x3100, "Mains Voltage" },
    { 0x3200, "Voltage inside the device" },
    { 0x3300, "Output Voltage" },
    { 0x4000, "Temperature" },
    { 0x4100, "Ambient Temperature" },
    { 0x4200, "Device Temperature" },
    { 0x5000, "Device Hardware" },
    { 0x6000, "Device Software" },
    { 0x6100, "Internal Software" },
    { 0x6200, "User Software" },
    { 0x6300, "Data Set" },
    { 0x7000, "Additional Modules" },
    { 0x8000, "Monitoring" },
    { 0x8100, "Communication" },
    { 0x8110, "CAN Overrun (Objects lost)" },
    { 0x8120, "CAN in Error (Passive Mode)" },
    { 0x8130, "Life Guard Error or Heartbeat Error" },
    { 0x8140, "Recovered from Bus-Off" },
    { 0x8150, "Transmit COB-ID collision" },
    { 0x8200, "Protocol Error" },
    { 0x8210, "PDO not processed due to length error" },
    { 0x8220, "PDO length exceeded" },
    { 0x9000, "External Error" },
    { 0xF000, "Additional Functions" },
    { 0xFF00, "Device Specific" }
};

const char* dict_lookup(uint16 index, uint8 sub_index)
{
    static const char* lookup;

    if (IS_TRUE == is_codb_loaded())
    {
        lookup = codb_desc_lookup(codb_get_profile(), index, sub_index);
        if (NULL != lookup)
        {
            return lookup;
        }
    }

    if (IS_TRUE == is_ds301_loaded())
    {
        lookup = codb_desc_lookup(codb_get_ds301_profile(), index, sub_index);
        if (NULL != lookup)
        {
            return lookup;
        }
    }

    return "";
}

status_t dict_lookup_object(uint16 index, uint8 sub_index)
{
    status_t      status = ALL_OK;
    object_info_t info;
    table_t       object_table;
    table_t       sub_index_table;
    const uint8   min_table_width = 14u;
    uint8         object_table_width;
    uint8         sub_index_table_width;

    const char* str[] =
    {
        ""
    };

    os_memset(&info, 0, sizeof(object_info_t));

    if (IS_TRUE == is_codb_loaded())
    {
        codb_info_lookup(codb_get_profile(), index, sub_index, &info);
    }

    if (IS_TRUE == is_ds301_loaded() && IS_FALSE == info.does_exist)
    {
        codb_info_lookup(codb_get_ds301_profile(), index, sub_index, &info);
    }

    if (IS_FALSE == info.does_exist)
    {
        return status;
    }

    object_table_width = os_strlen(info.name);
    if (object_table_width < min_table_width)
    {
        object_table_width = min_table_width;
    }

    object_table.frame_color    = DARK_CYAN;
    object_table.text_color     = DEFAULT_COLOR;
    object_table.column_a_width = 11;
    object_table.column_b_width = object_table_width;
    object_table.column_c_width = 1;

    status = table_init(&object_table, 1024);
    if (ALL_OK == status)
    {
        char buffer[CODB_MAX_DESC_LEN] = { 0 };

        os_printf("\nOBJECT DESCRIPTION");
        os_snprintf(buffer, sizeof(buffer), "%04Xh", index);

        table_print_header(&object_table);
        table_print_row("Index", buffer, "O", &object_table);

        os_snprintf(buffer, sizeof(buffer), "%u", info.entry_count);

        table_print_row("Elements", buffer, "B", &object_table);
        table_print_row("Name", info.name, "J", &object_table);

        os_snprintf(buffer, sizeof(buffer), "%s", object_code_lookup[info.code]);

        table_print_row("Object code", buffer, "E", &object_table);

        if (IS_VAR == info.code)
        {
            os_snprintf(buffer, sizeof(buffer), "%s", data_type_lookup[info.data_type]);
        }
        else
        {
            os_snprintf(buffer, sizeof(buffer), "-");
        }

        table_print_row("Data type", buffer, "C", &object_table);

        os_snprintf(buffer, sizeof(buffer), "%s", object_kind_lookup[info.category]);

        table_print_row("Category", buffer, "T", &object_table);
        table_print_footer(&object_table);
        table_flush(&object_table);
    }

    sub_index_table_width = os_strlen(info.sub_index_name);
    if (sub_index_table_width < min_table_width)
    {
        sub_index_table_width = min_table_width;
    }
    else if (0 == info.default_value)
    {
        sub_index_table_width = 33;
    }

    sub_index_table.frame_color    = DARK_CYAN;
    sub_index_table.text_color     = DEFAULT_COLOR;
    sub_index_table.column_a_width = 14;
    sub_index_table.column_b_width = sub_index_table_width;
    sub_index_table.column_c_width = 1;

    status = table_init(&sub_index_table, 1024);
    if (ALL_OK == status)
    {
        char buffer[CODB_MAX_DESC_LEN] = { 0 };
        os_printf("ENTRY DESCRIPTION");
        os_snprintf(buffer, sizeof(buffer), "%02Xh", sub_index);

        table_print_header(&sub_index_table);
        table_print_row("Sub-index", buffer, "E", &sub_index_table);
        table_print_row("Name", info.sub_index_name, "N", &sub_index_table);

        os_snprintf(buffer, sizeof(buffer), "%s", object_kind_lookup[info.entry_category]);

        table_print_row("Entry Category", buffer, "T", &sub_index_table);

        os_snprintf(buffer, sizeof(buffer), "%s", access_type_lookup[info.access_type]);

        table_print_row("Access", buffer, "R", &sub_index_table);

        if (SDL_FALSE == info.pdo_mapping)
        {
            os_snprintf(buffer, sizeof(buffer), "No");
        }
        else
        {
            os_snprintf(buffer, sizeof(buffer), "Optional");
        }

        table_print_row("PDO mapping", buffer, "Y", &sub_index_table);

        if (info.value_range_lower != 0 || info.value_range_upper != 0)
        {
            os_snprintf(buffer, sizeof(buffer), "%Xh - %Xh", info.value_range_lower, info.value_range_upper);
        }
        else
        {
            os_snprintf(buffer, sizeof(buffer), "%s", data_type_lookup[info.data_type]);
        }

        table_print_row("Value range", buffer, " ", &sub_index_table);

        if (info.default_value != 0)
        {
            os_snprintf(buffer, sizeof(buffer), "%Xh", info.default_value);
        }
        else
        {
            os_snprintf(buffer, sizeof(buffer), "Profile- or manufacturer-specific");
        }

        table_print_row("Default value", buffer, " ", &sub_index_table);
        table_print_footer(&sub_index_table);
        table_flush(&sub_index_table);
    }

    return status;
}

const char* dict_lookup_raw(can_message_t* message)
{
    static char buffer[1024] = { 0 };

    uint32 id;
    uint32 length;
    uint8* data;

    if (message == NULL)
    {
        return "";
    }

    id     = message->id;
    length = message->length;
    data   = message->data;

    /* NMT messages. */
    if ((0x0000 == id) && (2 == length))
    {
        switch (data[6])
        {
            case 0x01:
                return "NMT Start Remote Node";
            case 0x02:
                return "NMT Stop Remote Node";
            case 0x80:
                return "NMT Enter Pre-Operational";
            case 0x81:
                return "NMT Reset Node";
            case 0x82:
                return "NMT Reset Communication";
            default:
                return "";
        }
    }

    /* Heartbeat messages. */
    if ((id & 0x700) == 0x700 && 1 == length)
    {
        switch (data[7])
        {
            case 0x00:
                return "Boot-up Message";
            case 0x04:
                return "Heartbeat: Stopped.";
            case 0x05:
                return "Heartbeat: Operational";
            case 0x7F:
                return "Heartbeat: Pre-operational";
            default:
                return "";
        }
    }

    /* SDO messages. */
    if ((id & 0x600) == 0x600)
    {
        char* desc = NULL;
        uint16       index = (data[2] << 8) | data[1];
        uint8        sub_index = data[3];

        desc = (char*)dict_lookup(index, sub_index);
        if ('\0' == desc[0])
        {
            os_snprintf(buffer, sizeof(buffer), "SDO request, %04Xh sub %02Xh", index, sub_index);
        }
        else
        {
            os_snprintf(buffer, sizeof(buffer), "SDO request: %04Xh sub %02Xh, %s", index, sub_index, desc);
        }
        return buffer;
    }

    /* Abort codes. */
    if ((id & 0x580) == 0x580 && data[0] == ABORT_TRANSFER && 8 == length)
    {
        uint32 abort_code = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

        os_snprintf(buffer, sizeof(buffer), "SDO Abort Message %08Xh, %s", abort_code, sdo_lookup_abort_code(abort_code));
        return buffer;
    }
    else if ((id & 0x580) == 0x580)
    {
        return "SDO response";
    }

    /* PDO messages.
     * 0x181 - 0x1FF : PDO1 (tx)
     * 0x201 - 0x27F : PDO1 (rx)
     * 0x281 - 0x2FF : PDO2 (tx)
     * 0x301 - 0x37F : PDO2 (rx)
     * 0x381 - 0x3FF : PDO3 (tx)
     * 0x401 - 0x47F : PDO3 (rx)
     * 0x481 - 0x4FF : PDO4 (tx)
     * 0x501 - 0x57F : PDO4 (rx)
     */
    if ((id >= 0x181) && (id <= 0x1FF))
    {
        return "PDO1 (tx)";
    }
    else if ((id >= 0x201) && (id <= 0x27F))
    {
        return "PDO1 (rx)";
    }
    else if ((id >= 0x281) && (id <= 0x2FF))
    {
        return "PDO2 (tx)";
    }
    else if ((id >= 0x301) && (id <= 0x37F))
    {
        return "PDO2 (rx)";
    }
    else if ((id >= 0x381) && (id <= 0x3FF))
    {
        return "PDO3 (tx)";
    }
    else if ((id >= 0x401) && (id <= 0x47F))
    {
        return "PDO3 (rx)";
    }
    else if ((id >= 0x481) && (id <= 0x4FF))
    {
        return "PDO4 (tx)";
    }
    else if ((id >= 0x501) && (id <= 0x57F))
    {
        return "PDO4 (rx)";
    }

    /* EMCY messages. */
    if ((id & 0x080) == 0x080)
    {
        uint16 code = (data[1] << 8) | data[0];
        os_snprintf(buffer, sizeof(buffer), "EMCY %04Xh, %s", code, emcy_lookup(code));

        return buffer;
    }

    return "";
}

const char* emcy_lookup(uint16 code)
{
    size_t i;

    for (i = 0; i < sizeof(emcy_table) / sizeof(emcy_entry_t); ++i)
    {
        uint16 emcy_code = emcy_table[i].code;

        if (emcy_code == code)
        {
            return emcy_table[i].description;
        }
    }

    return "Unknown";
}
