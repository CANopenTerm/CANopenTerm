--[[ PCAN CAN TRC file player

Author:  Michael Fitzmayer
License: Public domain

--]]

local utils = require "lua/utils"

local function convert_data_bytes(data_bytes)
    local bytes = {}
    for byte in data_bytes:gmatch("%S+") do
        table.insert(bytes, byte)
    end
    
    while #bytes < 8 do
        table.insert(bytes, "00")
    end
    
    local data_d0_d3 = "0x" .. table.concat(bytes, "", 1, 4)
    local data_d4_d7 = "0x" .. table.concat(bytes, "", 5, 8)
    
    return data_d0_d3, data_d4_d7
end

function format_float(num)
    local formatted_str = string.format("%.1f", num)
    local length_needed = string.len("1234.5")
    
    if string.len(formatted_str) < length_needed then
        formatted_str = string.rep(" ", length_needed - string.len(formatted_str)) .. string.format("%.1f", num)
    end
    
    return formatted_str
end

function parse_pcan_trc(file_path)
    local trc_data = {}
    local file = io.open(file_path, "r")

    if not file then
        error("Could not open file: " .. file_path)
    end

    -- Read the first line to determine the file version
    local first_line = file:read("*line")
    local file_version = first_line:match("^;%$FILEVERSION=(%d%.%d)")

    if not file_version then
        error("Unknown file version: " .. first_line)
    end

    -- Define patterns for each version
    local patterns = {
        ["1.1"] = "^%s*%d+%)%s*([%d%.]+)%s+(%w+)%s+(%x+)%s+(%d)%s+(.+)$",
        ["1.3"] = "^%s*%d+%)%s*([%d%.]+)%s+%d%s+(%w+)%s+(%x+)%s+%-%s+(%d)%s+(.+)$"
    }

    local function parse_line(line, pattern)
        local time_offset, msg_type, can_id, dlc, data_bytes = line:match(pattern)

        if time_offset and msg_type and can_id and dlc and data_bytes then
            return {
                time_offset = tonumber(time_offset),
                msg_type = msg_type,
                can_id = can_id,
                dlc = tonumber(dlc),
                data_bytes = data_bytes:gsub("%s+", " ") -- Replace multiple spaces with a single space
            }
        else
            print("Line did not match pattern: " .. line)
        end
    end

    local pattern = patterns[file_version]

    if not pattern then
        error("Unsupported file version: " .. file_version)
    end

    for line in file:lines() do
        if not line:match("^;") then
            local message = parse_line(line, pattern)
            if message then
                table.insert(trc_data, message)
            else
                print("Failed to parse line: " .. line)
            end
        end
    end

    file:close()
    return trc_data
end

function round(x)
    return math.floor(x + 0.5)
end

function select_loop_count()
    io.write("\nHow often should the playback be looped? (or 'q' to quit): ")
    local choice = io.read()

    if choice == 'q' then
        return nil
    else
        choice = tonumber(choice)

        if choice >= 0 then
          return choice
        else
          print("Invalid number of loops. Please provide a non-negative integer.")
          select_loop_count()
        end
    end
end

function get_time()
    if os.getenv("OS") == "Windows_NT" then
        return os.clock()
    else
        return os.time()
    end
end

local num_loops = select_loop_count()
local trc_file  = nil

if num_loops == nil then
  print("Exiting.")
  return
end

trc_file = utils.get_file_by_selection("Enter the number of the file you want to choose", "trc")
if trc_file == nil then
  print("Exiting.")
  return
end

local base_name = trc_file:match("[^/\\]+$") or trc_file

for loop = 1, num_loops + 1 do
    local trc_data   = parse_pcan_trc(base_name)
    local start_time = get_time()
    local quit       = false

    for _, message in ipairs(trc_data) do
        if key_is_hit() then
          quit = true
          break
        end

        if message and message.time_offset and message.msg_type and message.can_id and message.dlc and message.data_bytes then
            local current_time = get_time()
            local elapsed_time = (current_time - start_time) * 1000
            local delay = math.floor(message.time_offset - elapsed_time)
            local data_d0_d3, data_d4_d7 = convert_data_bytes(message.data_bytes)

            if delay > 0 then
                delay_ms(delay)
            end

            can_write(tonumber(message.can_id, 16), message.dlc, data_d0_d3, data_d4_d7)
            print(string.format("Time Offset: %s, Msg Type: %s, CAN ID: %s, DLC: %d, Data Bytes: %s",
                format_float(message.time_offset), message.msg_type, message.can_id, message.dlc, message.data_bytes))
        else
            print("Invalid message format or nil value detected.")
            print("message: ", message)
        end
    end
    if quit == true then
      break
    end
end
