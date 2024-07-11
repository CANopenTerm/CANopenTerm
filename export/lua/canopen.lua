--[[

Title:   CANopen library
Author:  Michael Fitzmayer
License: Public domain

--]]

local function get_id_by_name(name)
  local available_nodes, total_devices = find_devices(2000)

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

return {
  get_id_by_name      = get_id_by_name,
  get_id_by_selection = get_id_by_selection,
}
