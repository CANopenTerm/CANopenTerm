--[[ TRC to CSV Export Script

Author:  Michael Fitzmayer
License: Public domain
Purpose: Export PCAN TRC trace files to CSV format for analysis in Excel/Calc.

This script helps field engineers:
- Convert binary trace data to human-readable CSV
- Analyze traces in spreadsheet applications
- Filter messages by ID or type
- Generate reports with time-based statistics
- Share data with non-engineers in readable format

--]]

local core = require "core"

local function parse_pcan_trc(file_path)
    local trc_data = {}
    local file = io.open(file_path, "r")

    if not file then
        error("Could not open file: " .. file_path)
    end

    local first_line = file:read("*line")
    local file_version = first_line:match("^;%$FILEVERSION=(%d%.%d)")

    if not file_version then
        error("Unknown file version: " .. first_line)
    end

    local patterns = {
        ["1.1"] = "^%s*%d+%)%s*([%d%.]+)%s+(%w+)%s+([%x]+)%s+(%d)%s+(.+)$",
        ["1.3"] = "^%s*%d+%)%s*([%d%.]+)%s+%d%s+(%w+)%s+([%x]+)%s+%-%s+(%d)%s+(.+)$",
        ["2.0"] = "^%s*%d+%s+([%d%.]+)%s+(%w+)%s+([%x]+)%s+%w+%s+(%d)%s+(.+)$",
    }

    local pattern = patterns[file_version]

    if not pattern then
        error("Unsupported file version: " .. file_version)
    end

    for line in file:lines() do
        if not line:match("^;") then
            local time_offset, msg_type, can_id, dlc, data_bytes = line:match(pattern)

            if time_offset and msg_type and can_id and dlc and data_bytes then
                table.insert(trc_data, {
                    time_offset = tonumber(time_offset),
                    msg_type = msg_type,
                    can_id = can_id,
                    dlc = tonumber(dlc),
                    data_bytes = data_bytes:gsub("%s+", " ")
                })
            end
        end
    end

    file:close()
    return trc_data
end

local function bytes_to_table(data_bytes_str, dlc)
    local bytes = {}
    for byte in data_bytes_str:gmatch("%S+") do
        table.insert(bytes, byte)
    end

    while #bytes < dlc do
        table.insert(bytes, "00")
    end

    return bytes
end

local function generate_csv(trc_data, output_file, filter_id)
    local file = assert(io.open(output_file, "w"))

    -- Write CSV header
    file:write("Time Offset (ms),Message Type,CAN ID (dec),CAN ID (hex),DLC")
    for i = 0, 7 do
        file:write(string.format(",Byte %d", i))
    end
    file:write("\n")

    -- Write data rows
    for _, msg in ipairs(trc_data) do
        local can_id_dec = tonumber(msg.can_id, 16)

        -- Apply filter if specified
        if filter_id == nil or can_id_dec == filter_id then
            file:write(string.format("%.3f,%s,%d,0x%s,%d",
                msg.time_offset,
                msg.msg_type,
                can_id_dec,
                msg.can_id,
                msg.dlc))

            local bytes = bytes_to_table(msg.data_bytes, msg.dlc)
            for i = 1, 8 do
                if i <= #bytes then
                    file:write("," .. bytes[i])
                else
                    file:write(",")
                end
            end
            file:write("\n")
        end
    end

    file:close()
end

-- Main execution
print("\n=== TRC to CSV Export ===\n")

local trc_path = ""
if os.getenv("OS") == "Windows_NT" then
    trc_path = os.getenv("USERPROFILE") .. "\\"
else
    trc_path = "/tmp/"
end

local trc_file = core.get_file_by_selection("Select TRC file to export", "trc", trc_path)
if trc_file == nil then
    print("Exiting.")
    return
end

print("Parsing TRC file...")
local trc_data = parse_pcan_trc(trc_file)
print(string.format("Loaded %d messages.\n", #trc_data))

local filter_choice = core.select_variable("Filter by specific CAN ID? [y/N]")
local filter_id = nil

if filter_choice and (filter_choice:lower() == "y" or filter_choice:lower() == "yes") then
    filter_id = core.select_number("Enter CAN ID to filter (decimal):")
    if filter_id == nil then
        print("No filter applied.")
    end
end

-- Generate output filename
local base_name = trc_file:match("[^/\\]+$")
local output_name = base_name:gsub("%.trc$", "") .. ".csv"

if os.getenv("OS") == "Windows_NT" then
    output_name = trc_path .. output_name
else
    output_name = "/tmp/" .. output_name
end

print(string.format("Exporting to CSV: %s", output_name))
generate_csv(trc_data, output_name, filter_id)

print(string.format("\n✓ Successfully exported to: %s\n", output_name))
