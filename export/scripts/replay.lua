--[[

Title:   CAN trace player
Author:  Michael Fitzmayer
License: Public domain

--]]

function list_trc_files()
    local files = {}
    local i = 1
    for file in io.popen('dir /b /a-d *.trc'):lines() do
        files[i] = file
        i = i + 1
    end
    return files
end

function choose_file(files)
    print("")
    for i, file in ipairs(files) do
        print(i .. ". " .. file)
    end
    io.write("\nEnter the number of the file you want to choose (or 'q' to quit): ")
    local choice = io.read()
    if choice == 'q' then
        return nil
    else
        choice = tonumber(choice)
        if choice and choice >= 1 and choice <= #files then
            return files[choice]
        else
            print("Invalid choice. Please enter a number between 1 and " .. #files .. " or 'q' to quit.")
            return choose_file(files)
        end
    end
end

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

    local function parse_line(line)
        local pattern = "^%s*%d+%)%s*([%d%.]+)%s+(%w+)%s+(%x+)%s+(%d)%s+(.+)$"
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

    for line in file:lines() do
        -- Skip comment lines
        if not line:match("^;") then
            local message = parse_line(line)
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

local files = list_trc_files()
if #files == 0 then
    print("No .trc files found in the current directory.")
else
    local chosen_file = choose_file(files)
    if chosen_file then
        local base_name = chosen_file:match("[^/\\]+$") or chosen_file

        print("Chosen file: " .. base_name)

        local trc_data = parse_pcan_trc(base_name)
        local start_time = os.clock()

        for _, message in ipairs(trc_data) do
            if key_is_hit() then
              break
            end

            if message and message.time_offset and message.msg_type and message.can_id and message.dlc and message.data_bytes then
                local current_time           = os.clock()
                local elapsed_time           = (current_time - start_time) * 1000
                local delay                  = math.floor(message.time_offset - elapsed_time)
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

    else
        print("Exiting script.")
    end
end
