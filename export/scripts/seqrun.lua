--[[ Run exported command sequence

Author:  Michael Fitzmayer
License: Public domain

--]]

local core = require "core"

local function read_seq_file(filepath)
    local commands = {}

    local file = io.open(filepath, "r")
    if not file then
        return nil
    end

    for line in file:lines() do
        -- Skip empty lines
        if line:match("%S") then
            table.insert(commands, line)
        end
    end
    file:close()

    return commands
end

-- Always run once
local num_loops = 0

-- Get .seq file path
local seq_path = ""
if os.getenv("OS") == "Windows_NT" then
    seq_path = os.getenv("USERPROFILE") .. "\\"
else
    seq_path = os.getenv("HOME") .. "/"
end

local seq_file = core.get_file_by_selection("Enter the number of the file you want to choose", "seq", seq_path)
if seq_file == nil then
    print("Exiting.")
    return
end

-- Read the sequence file
local commands = read_seq_file(seq_file)
if not commands then
    print(string.format("Error: Could not read sequence file at %s", seq_file))
    return
end

if #commands == 0 then
    print("Error: Sequence file is empty.")
    return
end

-- Ensure console is visible
console_show()

print()

-- Execute the sequence
run_sequence(table.unpack(commands))
