--[[

Title:   Device info (CiA 301)
Author:  Michael Fitzmayer
License: Public domain

--]]

local indices = {
    {index = 0x0000, description = "Static data types"},
    {index = 0x0001, start_sub_index = 0x00, description = "DEFTYPE BOOLEAN"},
    {index = 0x0002, start_sub_index = 0x00, description = "DEFTYPE INTEGER8"},
    {index = 0x0003, start_sub_index = 0x00, description = "DEFTYPE INTEGER16"},
    {index = 0x0004, start_sub_index = 0x00, description = "DEFTYPE INTEGER32"},
    {index = 0x0005, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED8"},
    {index = 0x0006, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED16"},
    {index = 0x0007, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED32"},
    {index = 0x0008, start_sub_index = 0x00, description = "DEFTYPE REAL32"},
    {index = 0x0009, start_sub_index = 0x00, description = "DEFTYPE VISIBLE_STRING"},
    {index = 0x000A, start_sub_index = 0x00, description = "DEFTYPE OCTET_STRING"},
    {index = 0x000B, start_sub_index = 0x00, description = "DEFTYPE UNICODE_STRING"},
    {index = 0x000C, start_sub_index = 0x00, description = "DEFTYPE TIME_OF_DAY"},
    {index = 0x000D, start_sub_index = 0x00, description = "DEFTYPE TIME_DIFFERENCE"},
    {index = 0x000F, start_sub_index = 0x00, description = "DEFTYPE DOMAIN"},
    {index = 0x0010, start_sub_index = 0x00, description = "DEFTYPE INTEGER24"},
    {index = 0x0011, start_sub_index = 0x00, description = "DEFTYPE REAL64"},
    {index = 0x0012, start_sub_index = 0x00, description = "DEFTYPE INTEGER40"},
    {index = 0x0013, start_sub_index = 0x00, description = "DEFTYPE INTEGER48"},
    {index = 0x0014, start_sub_index = 0x00, description = "DEFTYPE INTEGER56"},
    {index = 0x0015, start_sub_index = 0x00, description = "DEFTYPE INTEGER64"},
    {index = 0x0016, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED24"},
    {index = 0x0017, start_sub_index = 0x00, description = "reserved"},
    {index = 0x0018, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED40"},
    {index = 0x0019, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED48"},
    {index = 0x001A, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED56"},
    {index = 0x001B, start_sub_index = 0x00, description = "DEFTYPE UNSIGNED64"},
    {index = 0x001C, start_sub_index = 0x00, description = "reserved"},
    {index = 0x001D, start_sub_index = 0x00, description = "reserved"},
    {index = 0x001E, start_sub_index = 0x00, description = "reserved"},
    {index = 0x001F, start_sub_index = 0x00, description = "reserved"},

    {index = 0x0000, description = "Communication profile area"},
    {index = 0x1000, start_sub_index = 0x00, description = "Device type"},
    {index = 0x1001, start_sub_index = 0x00, description = "Error register"},
    {index = 0x1002, start_sub_index = 0x00, description = "Manufacturer status register"},
    {index = 0x1003, start_sub_index = 0x00, description = "Pre-defined error field, Number of errors"},
    {index = 0x1003, start_sub_index = 0x01, end_sub_index = 0xfe, description = "Pre-defined error field, Standard error field"},
    {index = 0x1003, start_sub_index = 0x02, description = "Pre-defined error field, Standard error field"},
    {index = 0x1005, start_sub_index = 0x00, description = "COB-ID SYNC"},
    {index = 0x1006, start_sub_index = 0x00, description = "Communication cycle period"},
    {index = 0x1007, start_sub_index = 0x00, description = "Synchronous window length"},
    {index = 0x1008, start_sub_index = 0x00, description = "Manufacturer device name"},
    {index = 0x1009, start_sub_index = 0x00, description = "Manufacturer hardware version"},
    {index = 0x100A, start_sub_index = 0x00, description = "Manufacturer software version"},
    {index = 0x100C, start_sub_index = 0x00, description = "Guard time"},
    {index = 0x100D, start_sub_index = 0x00, description = "Life time factor"},
    {index = 0x1010, start_sub_index = 0x00, description = "Store parameters, Highest supported sub-index"},
    {index = 0x1010, start_sub_index = 0x01, description = "Store parameters, All parameters that may be stored"},
    {index = 0x1010, start_sub_index = 0x02, description = "Store parameters, Communication related parameters (1000h to 1FFFh)"},
    {index = 0x1010, start_sub_index = 0x03, description = "Store parameters, Application related parameters (6000h to 9FFFh)"},
    {index = 0x1010, start_sub_index = 0x04, end_sub_index = 0x7F, description = "Store parameters, Manufacturer related parameters"},
    {index = 0x1010, start_sub_index = 0x80, end_sub_index = 0xFE, description = "Store parameters, reserved"},
    {index = 0x1011, start_sub_index = 0x00, description = "Restore parameters, Highest supported sub-index"},
    {index = 0x1011, start_sub_index = 0x01, description = "Restore parameters, All parameters that may be restored"},
    {index = 0x1011, start_sub_index = 0x02, description = "Restore parameters, Communication related parameters (1000h to 1FFFh)"},
    {index = 0x1011, start_sub_index = 0x03, description = "Restore parameters, Application related parameters (6000h to 9FFFh)"},
    {index = 0x1011, start_sub_index = 0x04, end_sub_index = 0x7F, description = "Store parameters, Manufacturer related parameters"},
    {index = 0x1011, start_sub_index = 0x80, end_sub_index = 0xFE, description = "Store parameters, reserved"},
    {index = 0x1012, start_sub_index = 0x00, description = "COB-ID time stamp message"},
    {index = 0x1013, start_sub_index = 0x00, description = "High resolution time stamp"},
    {index = 0x1014, start_sub_index = 0x00, description = "COB-ID EMCY"},
    {index = 0x1015, start_sub_index = 0x00, description = "Inhibit time EMCY"},
    {index = 0x1016, start_sub_index = 0x00, description = "Consumer heartbeat time, Highest supported sub-index"},
    {index = 0x1016, start_sub_index = 0x01, end_sub_index = 0x7F, description = "Consumer heartbeat time"},
    {index = 0x1017, start_sub_index = 0x00, description = "Producer heartbeat time"},
    {index = 0x1018, start_sub_index = 0x00, description = "Identity object, Highest supported sub-index"},
    {index = 0x1018, start_sub_index = 0x01, description = "Identity object, Vendor-ID"},
    {index = 0x1018, start_sub_index = 0x02, description = "Identity object, Product code"},
    {index = 0x1018, start_sub_index = 0x03, description = "Identity object, Revision number"},
    {index = 0x1018, start_sub_index = 0x04, description = "Identity object, Serial number"},
    {index = 0x1019, start_sub_index = 0x00, description = "Synchronous counter overflow value"},
    -- TBC.
}

local max_desc_length = 0
for _, entry in ipairs(indices) do
    local desc_length = #entry.description
    if desc_length > max_desc_length then
        max_desc_length = desc_length
    end
end

for _, entry in ipairs(indices) do
    local index = entry.index
    local start_sub_index = entry.start_sub_index
    local end_sub_index = entry.end_sub_index or start_sub_index

    if index == 0x00
    then
        print(string.format("\n %s", entry.description))
        print(" "..string.rep("-", string.len(entry.description)))
        print(string.format(" %-10s   %-10s   %-"..max_desc_length.."s   %-20s ", "Index", "Sub-Index", "Description", "Value"))
    else
        for sub_index = start_sub_index, end_sub_index do
            local value = sdo_read(0x40, index, sub_index)

            if value ~= nil then
                print(string.format(" %-10s   %-10s   %-"..max_desc_length.."s   %-20s ", string.format("0x%x", index), string.format("0x%x", sub_index), entry.description, value))
            end
        end
    end
end

print("")
