--[[ CANopen node scanner

Author:  Michael Fitzmayer
License: Public domain

--]]

local nodes      = {}
local start_time = os.clock()
local timeout_ms = 3000

nmt_send_command(0x00, 0x81) -- Reset all nodes.

while (os.clock() - start_time) * 1000 <= timeout_ms do
  local id, length, message = can_read()

  if key_is_hit() then
    break
  end

  -- Wait for boot-up messages.
  if length == 1 and message == 0x00 then
    table.insert(nodes, id - 0x700)
  end
end

for _, node_id in ipairs(nodes) do
  local temp, dev_name = sdo_read(node_id, 0x1008, 0x00)
  if nil == dev_name then
    dev_name = "Unknown device"
  end

  print_heading(dev_name)
  sdo_read(node_id,  0x1000, 0x00, true)
  sdo_read(node_id,  0x1009, 0x00, true)
  sdo_read(node_id,  0x100A, 0x00, true)
end

print("")
