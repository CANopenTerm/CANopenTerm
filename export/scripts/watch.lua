--[[ CAN DBC monitor

Author:  Michael Fitzmayer
License: Public domain

--]]

local utils = require "lua/utils"

local dbc_file = utils.get_file_by_selection("Select CAN DBC Database File", "dbc", "dbc/")
if dbc_file == nil then
  print("Exiting.")
  return
end

dbc_load(dbc_file)

local watch_id = utils.select_variable("Enter CAN-ID or provide a search term to find the ID by message name")

if type(watch_id) == "string" then
  if tonumber(watch_id) then
    watch_id = tonumber(watch_id)
  else
    watch_id = dbc_find_id_by_name(watch_id)
    if nil == watch_id then
      print("Exiting.")
      return
    end
  end
else
  print("Exiting.")
  return
end

local output    = dbc_decode(watch_id, 0x0000000000000000)
local num_lines = select(2, output:gsub('\n', '\n')) + 1

utils.clear_screen()

while false == key_is_hit() do
  local id, length, data = can_read()

  if id == watch_id then
    print(id)
    output = dbc_decode(watch_id, data)
    utils.print_multiline_at_same_position(output, num_lines)
  end
end
