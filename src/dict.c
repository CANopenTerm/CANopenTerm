/** @file dict.c
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include "SDL.h"
#include "core.h"
#include "lua.h"
#include "dict.h"

static const dict_entry_t dictionary[] =
{
    { 0x0000, 0x00, 0x00, "Static data types"},
    { 0x0001, 0x00, 0x00, "DEFTYPE BOOLEAN" },
    { 0x0002, 0x00, 0x00, "DEFTYPE INTEGER8" },
    { 0x0003, 0x00, 0x00, "DEFTYPE INTEGER16" },
    { 0x0004, 0x00, 0x00, "DEFTYPE INTEGER32" },
    { 0x0005, 0x00, 0x00, "DEFTYPE UNSIGNED8" },
    { 0x0006, 0x00, 0x00, "DEFTYPE UNSIGNED16" },
    { 0x0007, 0x00, 0x00, "DEFTYPE UNSIGNED32" },
    { 0x0008, 0x00, 0x00, "DEFTYPE REAL32" },
    { 0x0009, 0x00, 0x00, "DEFTYPE VISIBLE_STRING" },
    { 0x000A, 0x00, 0x00, "DEFTYPE OCTET_STRING" },
    { 0x000B, 0x00, 0x00, "DEFTYPE UNICODE_STRING" },
    { 0x000C, 0x00, 0x00, "DEFTYPE TIME_OF_DAY" },
    { 0x000D, 0x00, 0x00, "DEFTYPE TIME_DIFFERENCE" },
    { 0x000F, 0x00, 0x00, "DEFTYPE DOMAIN" },
    { 0x0010, 0x00, 0x00, "DEFTYPE INTEGER24" },
    { 0x0011, 0x00, 0x00, "DEFTYPE REAL64" },
    { 0x0012, 0x00, 0x00, "DEFTYPE INTEGER40" },
    { 0x0013, 0x00, 0x00, "DEFTYPE INTEGER48" },
    { 0x0014, 0x00, 0x00, "DEFTYPE INTEGER56" },
    { 0x0015, 0x00, 0x00, "DEFTYPE INTEGER64" },
    { 0x0016, 0x00, 0x00, "DEFTYPE UNSIGNED24" },
    { 0x0017, 0x00, 0x00, "Reserved" },
    { 0x0018, 0x00, 0x00, "DEFTYPE UNSIGNED40" },
    { 0x0019, 0x00, 0x00, "DEFTYPE UNSIGNED48" },
    { 0x001A, 0x00, 0x00, "DEFTYPE UNSIGNED56" },
    { 0x001B, 0x00, 0x00, "DEFTYPE UNSIGNED64" },
    { 0x001C, 0x00, 0x00, "Reserved" },
    { 0x001D, 0x00, 0x00, "Reserved" },
    { 0x001E, 0x00, 0x00, "Reserved" },
    { 0x001F, 0x00, 0x00, "Reserved" },
    { 0x0000, 0x00, 0x00, "Communication profile area" },
    { 0x1000, 0x00, 0x00, "Device type" },
    { 0x1001, 0x00, 0x00, "Error register" },
    { 0x1002, 0x00, 0x00, "Manufacturer status register" },
    { 0x1003, 0x00, 0x00, "Pre-defined error field, Number of errors" },
    { 0x1003, 0x01, 0xfe, "Pre-defined error field, Standard error field" },
    { 0x1003, 0x02, 0x00, "Pre-defined error field, Standard error field" },
    { 0x1005, 0x00, 0x00, "COB-ID SYNC" },
    { 0x1006, 0x00, 0x00, "Communication cycle period" },
    { 0x1007, 0x00, 0x00, "Synchronous window length" },
    { 0x1008, 0x00, 0x00, "Manufacturer device name" },
    { 0x1009, 0x00, 0x00, "Manufacturer hardware version" },
    { 0x100A, 0x00, 0x00, "Manufacturer software version" },
    { 0x100C, 0x00, 0x00, "Guard time" },
    { 0x100D, 0x00, 0x00, "Life time factor" },
    { 0x1010, 0x00, 0x00, "Store parameters, Highest sub-index supported" },
    { 0x1010, 0x01, 0x00, "Store parameters, All parameters that may be stored" },
    { 0x1010, 0x02, 0x00, "Store parameters, Communication related parameters (1000h to 1FFFh)" },
    { 0x1010, 0x03, 0x00, "Store parameters, Application related parameters (6000h to 9FFFh)" },
    { 0x1010, 0x04, 0x7F, "Store parameters, Manufacturer related parameters" },
    { 0x1010, 0x80, 0xFE, "Store parameters, reserved" },
    { 0x1011, 0x00, 0x00, "Restore parameters, Highest sub-index supported" },
    { 0x1011, 0x01, 0x00, "Restore parameters, All parameters that may be restored" },
    { 0x1011, 0x02, 0x00, "Restore parameters, Communication related parameters (1000h to 1FFFh)" },
    { 0x1011, 0x03, 0x00, "Restore parameters, Application related parameters (6000h to 9FFFh)" },
    { 0x1011, 0x04, 0x7F, "Store parameters, Manufacturer related parameters" },
    { 0x1011, 0x80, 0xFE, "Store parameters, reserved" },
    { 0x1012, 0x00, 0x00, "COB-ID time stamp message" },
    { 0x1013, 0x00, 0x00, "High resolution time stamp" },
    { 0x1014, 0x00, 0x00, "COB-ID EMCY" },
    { 0x1015, 0x00, 0x00, "Inhibit time EMCY" },
    { 0x1016, 0x00, 0x00, "Consumer heartbeat time, Highest sub-index supported" },
    { 0x1016, 0x01, 0x7F, "Consumer heartbeat time" },
    { 0x1017, 0x00, 0x00, "Producer heartbeat time" },
    { 0x1018, 0x00, 0x00, "Identity object, Highest sub-index supported" },
    { 0x1018, 0x01, 0x00, "Identity object, Vendor-ID" },
    { 0x1018, 0x02, 0x00, "Identity object, Product code" },
    { 0x1018, 0x03, 0x00, "Identity object, Revision number" },
    { 0x1018, 0x04, 0x00, "Identity object, Serial number" },
    { 0x1019, 0x00, 0x00, "Synchronous counter overflow value" },
    { 0x1020, 0x00, 0x00, "Verify configuration, Highest sub-index supported" },
    { 0x1020, 0x01, 0x00, "Verify configuration, Configuration date" },
    { 0x1020, 0x02, 0x00, "Verify configuration, Configuration time" },
    { 0x1021, 0x00, 0x00, "Store EDS" },
    { 0x1022, 0x00, 0x00, "Store format, /ISO10646/, not compressed" },
    { 0x1022, 0x01, 0x7F, "Store format, reserved" },
    { 0x1022, 0x80, 0xFF, "Store format, manufacturer-specific" },
    { 0x1023, 0x00, 0x00, "OS command, Highest sub-index supported" },
    { 0x1023, 0x01, 0x00, "OS command, Command" },
    { 0x1023, 0x02, 0x00, "OS command, Status" },
    { 0x1023, 0x03, 0x00, "OS command, Reply" },
    { 0x1024, 0x00, 0x00, "OS command mode" },
    { 0x1025, 0x00, 0x00, "OS debugger interface, Highest sub-index supported" },
    { 0x1025, 0x00, 0x00, "OS debugger interface, Command" },
    { 0x1025, 0x00, 0x00, "OS debugger interface, Status" },
    { 0x1025, 0x00, 0x00, "OS debugger interface, Reply" },
    { 0x1026, 0x00, 0x00, "OS prompt, Highest sub-index supported" },
    { 0x1026, 0x01, 0x00, "OS prompt, StdIn" },
    { 0x1026, 0x02, 0x00, "OS prompt, StdOut" },
    { 0x1026, 0x03, 0x00, "OS prompt, StdErr" },
    { 0x1027, 0x00, 0x00, "Module list, Number of connected modules" },
    { 0x1027, 0x01, 0x00, "Module list, Module 1" },
    { 0x1027, 0x02, 0xFE, "Module list, Module" },
    /* TBC. */
};

const char* dict_lookup(Uint16 index, Uint8 sub_index)
{
    size_t i;

    for (i = 0; i < sizeof(dictionary) / sizeof(dict_entry_t); ++i)
    {
        Uint8 start = dictionary[i].sub_index_start;
        Uint8 end   = dictionary[i].sub_index_end;

        if (end < start)
        {
            end = start;
        }

        if (dictionary[i].index == index &&
            sub_index >= start &&
            sub_index <= end)
        {
            return dictionary[i].description;
        }
    }

    return NULL;
}

int lua_dict_lookup(lua_State* L)
{
    int         index       = luaL_checkinteger(L, 1);
    int         sub_index   = luaL_checkinteger(L, 2);
    const char* description = dict_lookup(index, sub_index);

    if (NULL == description)
    {
        lua_pushnil(L);
    }
    else
    {
        lua_pushstring(L, description);
    }
    
    return 1;
}

void lua_register_dict_commands(core_t* core)
{
    lua_pushcfunction(core->L, lua_dict_lookup);
    lua_setglobal(core->L, "dict_lookup");
}
