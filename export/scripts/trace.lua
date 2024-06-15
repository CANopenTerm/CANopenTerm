--[[

Title:   CAN trace
Author:  Michael Fitzmayer
License: Public domain

--]]


local start_time_ms = os.time() * 1000 + os.clock() * 1000

print("Time    CAN-ID  Length  Data")

while not key_is_hit() do
  local id, length, data = can_read()

  if data then
    local current_time_ms = (os.time() * 1000 + os.clock() * 1000) - start_time_ms

    io.write(string.format("%06d  %03X     %1d       ", current_time_ms, id, length))

    for i = 1, length do
      io.write(string.format("%02X ", data:byte(i)))
    end

    io.write("\n")
  end
end
