--[[ Kinco RP2D Series Remote I/O

Author:  Michael Fitzmayer
License: Public domain

--]]

local canopen = require "lua/canopen"

print("Waiting for RP2D Series Remote I/O to send bootup message.")

local node_id = canopen.get_id_by_bootup_message(10000)
if node_id == nil then
    print("No bootup message received.\nExiting.")
    return
end

local vendor_id    = sdo_read(node_id, 0x1018, 1)
local product_code = sdo_read(node_id, 0x1018, 2)

if vendor_id == 0x300 then
  if product_code == 0x21231608 then
    print("RP2D-1608C1 with node ID " .. string.format("%Xh", 0x700 + node_id) .. " detected.")
  end
else
  print(string.format("%X", vendor_id))
end

print("Exiting.")
