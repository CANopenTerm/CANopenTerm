--[[ Export command sequence

Author:  Michael Fitzmayer
License: Public domain

--]]

local core = require "core"

local function get_history_filepath()
    if os.getenv("OS") == "Windows_NT" then
        local user_path = os.getenv("USERPROFILE")
        return user_path .. "\\" .. "CANopenTerm.history"
    else
        local home = os.getenv("HOME")
        return home .. "/.CANopenTerm.history"
    end
end

local function get_output_directory()
    if os.getenv("OS") == "Windows_NT" then
        return os.getenv("USERPROFILE")
    else
        return "/tmp"
    end
end

local function read_last_n_lines(filepath, n)
    local lines = {}

    local file = io.open(filepath, "r")
    if not file then
        return nil
    end

    for line in file:lines() do
        table.insert(lines, line)
    end
    file:close()

    -- Get last n lines
    local result = {}
    local start_index = math.max(1, #lines - n + 1)
    for i = start_index, #lines do
        table.insert(result, lines[i])
    end

    return result
end

-- Ask user for number of commands to save
local num_commands_str = core.select_variable("How many commands to save?")
if num_commands_str == nil then
    print("Exiting.")
    return
end

local num_commands = tonumber(num_commands_str)
if not num_commands or num_commands < 1 then
    print("Invalid number. Exiting.")
    return
end

-- Ask user for base filename
local base_filename = core.select_variable("Base filename for output (without .seq extension)")
if base_filename == nil then
    print("Exiting.")
    return
end

-- Remove .seq extension if user provided it
base_filename = base_filename:gsub("%.seq$", "")
if base_filename == "" then
    print("Invalid filename. Exiting.")
    return
end

-- Get history file path
local history_filepath = get_history_filepath()

-- Read history file - request N+1 to account for the seqsav command itself
local history_lines = read_last_n_lines(history_filepath, num_commands + 1)
if not history_lines then
    print(string.format("Error: Could not read history file at %s", history_filepath))
    return
end

-- Remove the last line if it contains "seqsav" (the command that was just executed)
if #history_lines > 0 then
    local last_line = history_lines[#history_lines]
    if last_line:match("seqsav") then
        table.remove(history_lines)
    end
end

-- Filter to only keep command calls (lines starting with single character and space)
local function is_command_call(line)
    return line:match("^%S ") ~= nil
end

local filtered_lines = {}
for _, line in ipairs(history_lines) do
    if is_command_call(line) then
        table.insert(filtered_lines, line)
    end
end
history_lines = filtered_lines

-- Warn if fewer commands are available than requested
if #history_lines < num_commands then
    print(string.format("Warning: Only %d command(s) available in history (requested %d)", #history_lines, num_commands))
end

-- Handle case when history is empty
if #history_lines == 0 then
    print("Error: No commands found in history file.")
    return
end

-- Construct output filepath
local output_dir = get_output_directory()
local output_filename = string.format("%s.seq", base_filename)
local output_filepath = output_dir .. (os.getenv("OS") == "Windows_NT" and "\\" or "/") .. output_filename

-- Write to .seq file
local success, err = pcall(function()
    local file = assert(io.open(output_filepath, "w"))

    for _, line in ipairs(history_lines) do
        file:write(line .. "\n")
    end

    file:close()
end)

if success then
    print(string.format("Saved %d command(s) to %s", #history_lines, output_filepath))
else
    print(string.format("Error writing to file: %s", err))
end
