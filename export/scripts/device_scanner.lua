--[[

Title:   Device scanner
Author:  Michael Fitzmayer
License: Public domain

--]]

local canopen = require "canopen"

local available_nodes, total_devices = canopen.find_devices(1000)

print_heading(string.format("Found %d device(s):", total_devices))

for _, node_id in ipairs(available_nodes) do
  sdo_read(node_id, 0x1000, 0x00, true)
  sdo_read(node_id, 0x1008, 0x00, true)
  sdo_read(node_id, 0x1009, 0x00, true)
  sdo_read(node_id, 0x100A, 0x00, true)
  sdo_read(node_id, 0x1018, 0x01, true)
  sdo_read(node_id, 0x1018, 0x02, true)
  sdo_read(node_id, 0x1018, 0x03, true)
  sdo_read(node_id, 0x1018, 0x04, true)  
end
