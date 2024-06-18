--[[

Title:   CAN trace
Author:  Michael Fitzmayer
License: Public domain

--]]

local initial_timestamp_us

print("Time         CAN-ID  Length  Data")

while not key_is_hit() do
  local id, length, data, timestamp_us = can_read()

  if data then
    if not initial_timestamp_us then
      initial_timestamp_us = timestamp_us
    end

    local elapsed_us = timestamp_us - initial_timestamp_us

    local timestamp_ms       = math.floor(elapsed_us / 1000)
    local timestamp_fraction = math.floor(((elapsed_us / 1000) %  1) * 1000)

    io.write(string.format("%6d.%03d   %03X     %1d       ", timestamp_ms, timestamp_fraction, id, length))

    for i = 1, length do
      io.write(string.format("%02X ", data:byte(i)))
    end

    io.write("\n")
  end
end
