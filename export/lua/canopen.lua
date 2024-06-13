--[[

Title:   CANopen library
Author:  Michael Fitzmayer
License: Public domain

--]]

local function find_devices(timeout_ms)
    local start_time = os.clock()
    local timeout = timeout_ms / 1000
    local node_ids = {}

    while os.clock() - start_time < timeout do
        local id, length, message = can_read()

        if id then
            if id >= 0x701 and id <= 0x77F then
                local node_id = id - 0x700

                if not node_ids[node_id] then
                    node_ids[node_id] = true
                end
            end
        end
    end

    local node_list = {}
    for k in pairs(node_ids) do
        table.insert(node_list, k)
    end

    local total_devices = #node_list

    return node_list, total_devices
end

local function get_id_by_name(name)
  local available_nodes, total_devices = find_devices(1000)

  if total_devices >= 1 then
    for _, node_id in ipairs(available_nodes) do
      local device_name = sdo_read(node_id, 0x1008, 0x00)
      if device_name == device_name then
        return node_id
      end
    end
  end

  return 0
end

return { find_devices = find_devices, get_id_by_name = get_id_by_name }
