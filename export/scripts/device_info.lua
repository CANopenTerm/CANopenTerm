--[[

Title:   Device info
Author:  Michael Fitzmayer
License: Public domain

--]]

print("Enter node ID:")
io.write(": ")
local node_id = io.read("*n")

if (node_id < 1 or node_id > 127) then
    node_id = 0x40
end

print_heading("Device information")
sdo_read(node_id, 0x1000, 0x00, true)
sdo_read(node_id, 0x1008, 0x00, true)
sdo_read(node_id, 0x1009, 0x00, true)
sdo_read(node_id, 0x100A, 0x00, true)
sdo_read(node_id, 0x1018, 0x01, true)
sdo_read(node_id, 0x1018, 0x02, true)
sdo_read(node_id, 0x1018, 0x03, true)
sdo_read(node_id, 0x1018, 0x04, true)
