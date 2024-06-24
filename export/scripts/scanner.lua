--[[ CANopen node scanner

Author:  Michael Fitzmayer
License: Public domain

--]]

local canopen = require "lua/canopen"

local available_nodes, total_devices = canopen.find_devices(1000)

print(string.format("\nFound %d device(s)", total_devices))

for _, node_id in ipairs(available_nodes) do
  local device_name = sdo_read(node_id, 0x4555, 0x05)
  if nil == device_name then
    device_name = "Unknown device"
  end

  print_heading(device_name)
  sdo_read(node_id, 0x1000, 0x00, true)
  sdo_read(node_id, 0x1009, 0x00, true)
  sdo_read(node_id, 0x100A, 0x00, true)
end

print("")
