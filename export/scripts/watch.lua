--[[ CAN DBC monitor

Author:  Michael Fitzmayer
License: Public domain

--]]

local utils = require "lua/utils"

function clear_screen()
    if package.config:sub(1,1) == '\\' then
        -- Windows system
        os.execute('cls')
    else
        -- Unix-based system
        os.execute('clear')
    end
end

function print_multiline_at_same_position(message, num_lines)
    local buffer = ""

    for _ = 1, num_lines do
        buffer = buffer .. "\27[2K"
        buffer = buffer .. "\27[A"
    end

    buffer = buffer .. "\r"
    buffer = buffer .. message

    io.write(buffer)
    io.flush()
end

dbc_file = utils.get_file_by_selection("Select CAN DBC DatabaseFile", "dbc")
if dbc_file == nil then
  print("Exiting.")
  return
end

dbc_load(dbc_file)

local output    = dbc_decode(466, 0x00000000, 0x00000000)
local num_lines = select(2, output:gsub('\n', '\n')) + 1

local demo = 0

clear_screen()

while false == key_is_hit() do
  demo = demo + 1

  if demo >= 0xffffffff then
    demo = 0
  end

  output = dbc_decode(466, demo, demo)
  print_multiline_at_same_position(output, num_lines)
end
