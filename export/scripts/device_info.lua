--[[

Title:   Device info (CiA 301)
Author:  Michael Fitzmayer
License: Public domain

--]]

local indices = {
    -- Static data types
    {index = 0x0001, sub_index = 0x00, description = "DEFTYPE BOOLEAN"},
    {index = 0x0002, sub_index = 0x00, description = "DEFTYPE INTEGER8"},
    {index = 0x0003, sub_index = 0x00, description = "DEFTYPE INTEGER16"},
    {index = 0x0004, sub_index = 0x00, description = "DEFTYPE INTEGER32"},
    {index = 0x0005, sub_index = 0x00, description = "DEFTYPE UNSIGNED8"},
    {index = 0x0006, sub_index = 0x00, description = "DEFTYPE UNSIGNED16"},
    {index = 0x0007, sub_index = 0x00, description = "DEFTYPE UNSIGNED32"},
    {index = 0x0008, sub_index = 0x00, description = "DEFTYPE REAL32"},
    {index = 0x0009, sub_index = 0x00, description = "DEFTYPE VISIBLE_STRING"},
    {index = 0x000A, sub_index = 0x00, description = "DEFTYPE OCTET_STRING"},
    {index = 0x000B, sub_index = 0x00, description = "DEFTYPE UNICODE_STRING"},
    {index = 0x000C, sub_index = 0x00, description = "DEFTYPE TIME_OF_DAY"},
    {index = 0x000D, sub_index = 0x00, description = "DEFTYPE TIME_DIFFERENCE"},
    {index = 0x000F, sub_index = 0x00, description = "DEFTYPE DOMAIN"},
    {index = 0x0010, sub_index = 0x00, description = "DEFTYPE INTEGER24"},
    {index = 0x0011, sub_index = 0x00, description = "DEFTYPE REAL64"},
    {index = 0x0012, sub_index = 0x00, description = "DEFTYPE INTEGER40"},
    {index = 0x0013, sub_index = 0x00, description = "DEFTYPE INTEGER48"},
    {index = 0x0014, sub_index = 0x00, description = "DEFTYPE INTEGER56"},
    {index = 0x0015, sub_index = 0x00, description = "DEFTYPE INTEGER64"},
    {index = 0x0016, sub_index = 0x00, description = "DEFTYPE UNSIGNED24"},
    {index = 0x0018, sub_index = 0x00, description = "DEFTYPE UNSIGNED40"},
    {index = 0x0019, sub_index = 0x00, description = "DEFTYPE UNSIGNED48"},
    {index = 0x001A, sub_index = 0x00, description = "DEFTYPE UNSIGNED56"},
    {index = 0x001B, sub_index = 0x00, description = "DEFTYPE UNSIGNED64"},
    -- Communication profile area
    {index = 0x1000, sub_index = 0x00, description = "Device type"},
    {index = 0x1001, sub_index = 0x00, description = "Error register"},
    {index = 0x1002, sub_index = 0x00, description = "Manufacturer status register"},
    -- Tbc.
}

local max_desc_length = 0
for _, entry in ipairs(indices) do
    local desc_length = #entry.description
    if desc_length > max_desc_length then
        max_desc_length = desc_length
    end
end

print("")
print(string.format(" %-10s   %-10s   %-"..max_desc_length.."s   %-20s ", "Index", "Sub-Index", "Description", "Value"))

for _, entry in ipairs(indices) do
    local index = entry.index
    local sub_index = entry.sub_index
    local description = entry.description

    local value = sdo_read(0x40, index, sub_index)

    print(string.format(" %-10s   %-10s   %-"..max_desc_length.."s   %-20s ", string.format("0x%x", index), string.format("0x%x", sub_index), description, value or "Read failed"))
end

print("")
