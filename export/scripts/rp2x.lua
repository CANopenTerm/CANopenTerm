--[[ Kinco RP2x Series Remote I/O

Author:  Michael Fitzmayer
License: Public domain

--]]

local canopen   = require "lua/canopen"
local max_nodes = 1;

print("Waiting for RP2x Series Remote I/O to send bootup messages.")

local node_list = canopen.get_ids_by_bootup_message(5000, max_nodes)
if node_list == nil then
    print("No bootup message received.\nExiting.")
    return
end

for i = 1, #node_list do
  local vendor_id    = sdo_read(node_list[i], 0x1018, 1)
  local product_code = sdo_read(node_list[i], 0x1018, 2)

  if vendor_id == 0x300 then
    if product_code == 0x21231608 then
      print("RP2D-1608C1 with node ID " .. string.format("%Xh", 0x700 + node_list[i]) .. " detected.")
    elseif product_code == 0x21231616 then
      print("RP2D-0016C1 with node ID " .. string.format("%Xh", 0x700 + node_list[i]) .. " detected.")
    elseif product_code == 0x21231632 then
      print("RP2A-0402C1 with node ID " .. string.format("%Xh", 0x700 + node_list[i]) .. " detected.")
    else
      table.remove(node_list, i)
    end
  else
    table.remove(node_list, i)
  end
end

for i = 1, #node_list do
  sdo_write(node_list[i], 0x1017, 0, 2, 1000) -- Set heartbeat time to 1000ms
  nmt_send_command(node_list[i], 0x01)        -- Start node)
end

print("Exiting.")
