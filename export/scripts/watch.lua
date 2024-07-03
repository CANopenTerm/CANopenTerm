--[[ CAN DBC monitor

Author:  Michael Fitzmayer
License: Public domain

--]]

local utils = require "lua/utils"

dbc_file = utils.get_file_by_selection("Select CAN DBC DatabaseFile", "dbc")
if dbc_file == nil then
  print("Exiting.")
  return
end

dbc_load(dbc_file)

local output    = dbc_decode(466, 0x00000000, 0x00000000)
local num_lines = select(2, output:gsub('\n', '\n')) + 1

local demo = 0

utils.clear_screen()

while false == key_is_hit() do
  demo = demo + 1

  if demo >= 0xffffffff then
    demo = 0
  end

  output = dbc_decode(466, demo, demo)
  utils.print_multiline_at_same_position(output, num_lines)
end
