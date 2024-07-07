--[[

Title:   CANopen library
Author:  Michael Fitzmayer
License: Public domain

--]]

local function find_devices(timeout_ms)
  local start_time = os.time()
  local timeout    = timeout_ms / 1000  -- convert milliseconds to seconds
  local node_ids   = {}

  while os.time() - start_time < timeout do
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

local function get_id_by_bootup_message(timeout_ms)
  local start_time = os.time()
  local timeout    = timeout_ms / 1000  -- convert milliseconds to seconds

  while os.time() - start_time < timeout do
    local id, length, message = can_read()

    if length == 1 and message == 0x00 then
      return id - 0x700
    end
  end

  return nil
end

local function get_ids_by_bootup_message(timeout_ms, max_nodes)
  local node_count = 0
  local node_list  = {}
  local start_time = os.time()
  local timeout    = timeout_ms / 1000  -- convert milliseconds to seconds

  while os.time() - start_time < timeout do
    local id = get_id_by_bootup_message(timeout_ms)

    if id then
      table.insert(node_list, id)
      node_count = node_count + 1
    end

    if node_count >= max_nodes then
      break
    end
  end

  return node_list
end

local function get_id_by_name(name)
  local available_nodes, total_devices = find_devices(1000)

  if total_devices >= 1 then
    for _, node_id in ipairs(available_nodes) do
      local device_name = sdo_read(node_id, 0x1008, 0x00)
      if device_name == name then
        return node_id
      end
    end
  end

  return 0
end

local function get_id_by_selection(prompt)
  print("")

  local available_nodes, total_devices = find_devices(1000)
  if total_devices == 0 then
    print("No devices found.")
    return nil
  end

  for i, node in ipairs(available_nodes) do
    print(i .. ". " .. sdo_read(node, 0x1008, 0))
  end

  io.write("\n" .. prompt .." (or 'q' to quit): ")
  local choice = io.read()

  if choice == 'q' then
    return nil
  else
    choice = tonumber(choice)
    if choice and choice >= 1 and choice <= #available_nodes then
      return available_nodes[choice]
    else
      print("Invalid choice. Please enter a number between 1 and " .. #available_nodes .. " or 'q' to quit.")
      return get_id_by_selection(prompt)
    end
  end
end

local function node_reset(node_id)
  nmt_send_command(node_id, 0x81) -- Reset
  delay_ms(100)
  
  while true do
    local can_id, data_len, data = can_read()
    if can_id ~= nil then
      if can_id == 0x700 + node_id then
        if length == 1 and message == 0x00 then
          break
        end
      end
    end
    
    if key_is_hit() then
      break;
    end
  end
end

return {
  find_devices              = find_devices,
  get_id_by_bootup_message  = get_id_by_bootup_message,
  get_ids_by_bootup_message = get_ids_by_bootup_message,
  get_id_by_name            = get_id_by_name,
  get_id_by_selection       = get_id_by_selection,
  node_reset                = node_reset
}
