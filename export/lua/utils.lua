--[[

Title:   Utility functions
Author:  Michael Fitzmayer
License: Public domain

--]]

function clear_screen()
    if package.config:sub(1,1) == '\\' then
        -- Windows system
        os.execute('cls')
    else
        -- Unix-based system
        os.execute('clear')
    end
end

local function get_file_list(extension, directory)
  directory = directory or ""

  -- Convert directory path separators based on platform
  local sep = package.config:sub(1, 1)  -- Get platform-specific path separator
  directory = directory:gsub("/", sep)  -- Replace forward slashes with platform-specific separator

  local files = {}
  local i = 1
  local command

  if os.getenv("OS") == "Windows_NT" then
    command = 'dir /b /a-d "' .. directory .. '*.' .. extension .. '" 2>nul'
  else
    command = 'ls -1 "' .. directory .. '*.' .. extension .. '" 2>/dev/null'
  end

  for file in io.popen(command):lines() do
    files[i] = file
    i = i + 1
  end

  if #files == 0 then
    print("No ." .. extension .. " files found in directory: " .. directory)
    return nil
  end

  return files
end

local function get_file_by_selection(prompt, extension, sub_directory)
  print("")

  local directory = sub_directory or ""

  -- Convert directory path separators based on platform
  local sep = package.config:sub(1, 1)  -- Get platform-specific path separator
  directory = directory:gsub("/", sep)  -- Replace forward slashes with platform-specific separator

  local files = get_file_list(extension, directory)

  if files == nil then
    print("Exiting.")
    return nil
  end

  for i, file in ipairs(files) do
    print(i .. ". " .. file)
  end

  io.write("\n" .. prompt .. " (or 'q' to quit): ")
  local choice = io.read()

  if choice == 'q' then
    return nil
  else
    choice = tonumber(choice)
    if choice and choice >= 1 and choice <= #files then
      -- Convert back to original directory path separators for returning
      local original_sep = sub_directory and "/" or sep  -- Determine if sub_directory was provided
      return directory:gsub(sep, original_sep) .. files[choice]
    else
      print("Invalid choice. Please enter a number between 1 and " .. #files .. " or 'q' to quit.")
      return get_file_by_selection(prompt, extension, sub_directory)
    end
  end
end

function print_multiline_at_same_position(message, num_lines)
    local buffer = ""

    for _ = 1, num_lines do
        buffer = buffer .. "\27[2K"
        buffer = buffer .. "\27[A"
    end

    buffer = buffer .. "\r"
    buffer = buffer .. message

    io.write(buffer)
    io.flush()
end

function select_number(prompt)
  io.write("\n" .. prompt .. " (or 'q' to quit): ")
  local choice = io.read()

  if choice == 'q' then
    return nil
  else
    choice = tonumber(choice)

    if choice >= 0 then
      return choice
    else
      print("Please provide a non-negative integer.")
      select_number()
    end
  end
end

local function read_word(file)
    local bytes = file:read(2)
    if not bytes or #bytes < 2 then
        error("Failed to read 2 bytes for WORD")
    end
    local b1, b2 = string.byte(bytes, 1, 2)
    return b1 + b2 * 256
end

local function read_long(file)
    local bytes = file:read(4)
    if not bytes or #bytes < 4 then
        error("Failed to read 4 bytes for LONG")
    end
    local b1, b2, b3, b4 = string.byte(bytes, 1, 4)
    return b1 + b2 * 256 + b3 * 65536 + b4 * 16777216
end

local function read_ulong(file)
    local bytes = file:read(4)
    if not bytes or #bytes < 4 then
        error("Failed to read 4 bytes for ULONG")
    end
    local b1, b2, b3, b4 = string.byte(bytes, 1, 4)
    return b1 + b2 * 256 + b3 * 65536 + b4 * 16777216
end

local function read_byte(file)
    local byte = file:read(1)
    if not byte or #byte < 1 then
        error("Failed to read 1 byte")
    end
    return string.byte(byte)
end

local function left_shift(a, shift)
    return a * 2^shift
end

local function is_bit_set(number, position)
    return bitwise_and(number, left_shift(1, position)) ~= 0
end

local function bitwise_and(a, b)
    local result = 0
    local bit_val = 1
    while a > 0 and b > 0 do
        local ra = a % 2
        local rb = b % 2
        if ra == 1 and rb == 1 then
            result = result + bit_val
        end
        bit_val = bit_val * 2
        a = (a - ra) / 2
        b = (b - rb) / 2
    end
    return result
end

function swap_bytes(data, length)
    local swapped = 0
    for i = 0, length-1 do
        local byte = (data >> (i * 8)) & 0xFF
        swapped = swapped | (byte << ((length - 1 - i) * 8))
    end
    return swapped
end

return {
  clear_screen                     = clear_screen,
  get_file_list                    = get_file_list,
  get_file_by_selection            = get_file_by_selection,
  print_multiline_at_same_position = print_multiline_at_same_position,
  select_number                    = select_number,
  read_word                        = read_word,
  read_long                        = read_long,
  read_ulong                       = read_ulong,
  read_byte                        = read_byte,
  left_shift                       = left_shift,
  is_bit_set                       = is_bit_set,
  bitwise_and                      = bitwise_and,
  swap_bytes                       = swap_bytes
}
