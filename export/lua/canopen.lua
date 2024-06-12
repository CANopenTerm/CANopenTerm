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

return { find_devices = find_devices }
