--[[ STM32 CAN Bootloader Protocol

Author:  Michael Fitzmayer
License: Public domain
Comment: Interactive implementation of ST's AN3154 CAN bootloader
         protocol for STM32 microcontrollers. Supports firmware
         updates, memory operations, and device management over
         CAN bus.

--]]

local core = require "core"

-- AN3154 Command Set
local CMD = {
    GET             = 0x00,  -- Get version and allowed commands
    GET_VERSION     = 0x01,  -- Get version and read protection status
    GET_ID          = 0x02,  -- Get chip ID
    READ_MEMORY     = 0x11,  -- Read memory
    GO              = 0x21,  -- Jump to address
    WRITE_MEMORY    = 0x31,  -- Write memory
    ERASE           = 0x43,  -- Erase memory (legacy)
    EXTENDED_ERASE  = 0x44,  -- Extended erase memory
    WRITE_PROTECT   = 0x63,  -- Enable write protection
    WRITE_UNPROTECT = 0x73,  -- Disable write protection
    READOUT_PROTECT = 0x82,  -- Enable readout protection
    READOUT_UNPROTECT = 0x92, -- Disable readout protection
}

local CMD_NAMES = {
    [CMD.GET]             = "Get",
    [CMD.GET_VERSION]     = "Get Version",
    [CMD.GET_ID]          = "Get ID",
    [CMD.READ_MEMORY]     = "Read Memory",
    [CMD.GO]              = "Go",
    [CMD.WRITE_MEMORY]    = "Write Memory",
    [CMD.ERASE]           = "Erase",
    [CMD.EXTENDED_ERASE]  = "Extended Erase",
    [CMD.WRITE_PROTECT]   = "Write Protect",
    [CMD.WRITE_UNPROTECT] = "Write Unprotect",
    [CMD.READOUT_PROTECT] = "Readout Protect",
    [CMD.READOUT_UNPROTECT] = "Readout Unprotect",
}

-- Response codes
local ACK  = 0x79
local NACK = 0x1F

-- Configuration
local config = {
    master_id = 0x111,  -- Master (PC) CAN ID
    slave_id  = 0x222,  -- Slave (STM32) CAN ID
    timeout   = 1000,   -- Response timeout in ms
    use_extended_id = false, -- Use 11-bit standard IDs by default
}

-- Statistics
local stats = {
    commands_sent = 0,
    acks_received = 0,
    nacks_received = 0,
    timeouts = 0,
    bytes_read = 0,
    bytes_written = 0,
}

-- Known STM32 chip IDs
local CHIP_IDS = {
    [0x0410] = "STM32F10x Medium-density",
    [0x0411] = "STM32F2xx",
    [0x0412] = "STM32F10x Low-density",
    [0x0413] = "STM32F40x/41x",
    [0x0414] = "STM32F10x High-density",
    [0x0415] = "STM32L4xx",
    [0x0416] = "STM32L1xx Medium-density",
    [0x0417] = "STM32L0xx",
    [0x0418] = "STM32F10x Connectivity line",
    [0x0419] = "STM32F42x/43x",
    [0x0420] = "STM32F10x Medium-density value line",
    [0x0421] = "STM32F446",
    [0x0422] = "STM32F30x/31x",
    [0x0423] = "STM32F401xB/C",
    [0x0425] = "STM32L0xx Cat.2",
    [0x0427] = "STM32L1xx Medium-density Plus",
    [0x0428] = "STM32F10x High-density value line",
    [0x0429] = "STM32L1xx Cat.2",
    [0x0430] = "STM32F10x XL-density",
    [0x0431] = "STM32F411",
    [0x0432] = "STM32F37x",
    [0x0433] = "STM32F401xD/E",
    [0x0434] = "STM32F469/479",
    [0x0435] = "STM32L43x/44x",
    [0x0436] = "STM32L1xx High-density",
    [0x0437] = "STM32L152RE",
    [0x0438] = "STM32F334",
    [0x0439] = "STM32F302xB/C",
    [0x0440] = "STM32F0xx",
    [0x0441] = "STM32F412",
    [0x0442] = "STM32F09x/030x8",
    [0x0444] = "STM32F03x",
    [0x0445] = "STM32F04x/070x6",
    [0x0446] = "STM32F303xD/E",
    [0x0447] = "STM32L0xx Cat.5",
    [0x0448] = "STM32F07x/070xB",
    [0x0449] = "STM32F74x/75x",
    [0x0450] = "STM32H74x/75x",
    [0x0451] = "STM32F76x/77x",
    [0x0452] = "STM32F72x/73x",
    [0x0457] = "STM32L0xx Cat.3",
    [0x0458] = "STM32F410",
    [0x0460] = "STM32G0xx",
    [0x0461] = "STM32L496/4A6",
    [0x0462] = "STM32L45x/46x",
    [0x0463] = "STM32F413/423",
    [0x0464] = "STM32L41x/42x",
    [0x0466] = "STM32G03x/04x",
    [0x0468] = "STM32G4xx Cat.2",
    [0x0469] = "STM32G4xx Cat.3",
    [0x0470] = "STM32L4Rx/4Sx",
    [0x0471] = "STM32L4P5/4Q5",
    [0x0472] = "STM32L55x/56x",
    [0x0479] = "STM32G4xx Cat.4",
    [0x0480] = "STM32H7Ax/7Bx",
    [0x0483] = "STM32H72x/73x",
    [0x0495] = "STM32WBxx",
    [0x0497] = "STM32WLxx",
}

-- ============================================================
-- Helper Functions
-- ============================================================

-- Pack bytes into uint64 for can_write (big-endian)
local function pack_bytes(...)
    local bytes = {...}
    local data = 0
    for i = 1, math.min(#bytes, 8) do
        data = data | (bytes[i] << ((8 - i) * 8))
    end
    return data
end

-- Extract byte at position n (0-based, MSB first) from uint64
local function byte_at(data, n)
    return (data >> ((7 - n) * 8)) & 0xFF
end

-- Format data as hex string
local function format_hex(data, length)
    local bytes = {}
    for i = 0, length - 1 do
        local byte = byte_at(data, i)
        table.insert(bytes, string.format("%02X", byte))
    end
    return table.concat(bytes, " ")
end

-- Calculate XOR checksum
local function calc_checksum(...)
    local bytes = {...}
    local checksum = 0
    for _, byte in ipairs(bytes) do
        checksum = checksum ~ byte
    end
    return checksum
end

-- Wait for response from bootloader
local function wait_for_response(expected_length)
    local deadline = os.clock() + config.timeout / 1000
    local response_data = {}

    can_flush()

    while os.clock() < deadline do
        local id, length, data = can_read()

        if id == config.slave_id and length and data then
            -- Collect response bytes
            for i = 0, length - 1 do
                table.insert(response_data, byte_at(data, i))
            end

            -- Check if we have enough data
            if not expected_length or #response_data >= expected_length then
                return response_data
            end
        end

        delay_ms(1)
    end

    stats.timeouts = stats.timeouts + 1
    return nil
end

-- Send command and wait for ACK
local function send_command(cmd_byte, show_details)
    local checksum = calc_checksum(cmd_byte, 0xFF - cmd_byte)
    local data = pack_bytes(cmd_byte, 0xFF - cmd_byte)

    if show_details then
        print(string.format("  Sending command: %s (0x%02X)", 
            CMD_NAMES[cmd_byte] or "Unknown", cmd_byte))
    end

    can_write(config.master_id, 2, data, false)
    stats.commands_sent = stats.commands_sent + 1

    local response = wait_for_response(1)

    if response and response[1] == ACK then
        stats.acks_received = stats.acks_received + 1
        if show_details then
            print("  ACK received")
        end
        return true
    elseif response and response[1] == NACK then
        stats.nacks_received = stats.nacks_received + 1
        if show_details then
            print("  NACK received")
        end
        return false
    else
        if show_details then
            print("  Timeout waiting for response")
        end
        return false
    end
end

-- Send data with checksum
local function send_data(bytes, show_details)
    local checksum = calc_checksum(table.unpack(bytes))
    table.insert(bytes, checksum)

    -- Send in chunks of 8 bytes
    local offset = 1
    while offset <= #bytes do
        local chunk = {}
        for i = 0, 7 do
            if offset + i <= #bytes then
                table.insert(chunk, bytes[offset + i])
            end
        end

        local data = pack_bytes(table.unpack(chunk))
        can_write(config.master_id, #chunk, data, false)
        offset = offset + 8
        delay_ms(5)
    end

    -- Wait for ACK
    local response = wait_for_response(1)

    if response and response[1] == ACK then
        stats.acks_received = stats.acks_received + 1
        if show_details then
            print("  ACK received")
        end
        return true
    elseif response and response[1] == NACK then
        stats.nacks_received = stats.nacks_received + 1
        if show_details then
            print("  NACK received")
        end
        return false
    else
        if show_details then
            print("  Timeout")
        end
        return false
    end
end

-- ============================================================
-- AN3154 Protocol Commands
-- ============================================================

-- Get version and available commands
function cmd_get()
    print("\n" .. string.rep("─", 60))
    print("GET - Get bootloader version and available commands")
    print(string.rep("─", 60))

    if not send_command(CMD.GET, true) then
        print("  Command failed")
        return false
    end

    -- Read response: N + 1 bytes (version + N command codes) + ACK
    local response = wait_for_response()

    if not response or #response < 2 then
        print("  Invalid response")
        return false
    end

    local n = response[1]
    local version = response[2]

    print(string.format("\nBootloader Version: 0x%02X (v%d.%d)", 
        version, (version >> 4) & 0x0F, version & 0x0F))
    print(string.format("Number of commands: %d", n))

    if #response >= n + 2 then
        print("\nSupported commands:")
        for i = 3, n + 2 do
            local cmd = response[i]
            print(string.format("  0x%02X - %s", cmd, CMD_NAMES[cmd] or "Unknown"))
        end
    end

    return true
end

-- Get version and read protection status
function cmd_get_version()
    print("\n" .. string.rep("─", 60))
    print("GET VERSION - Get bootloader version and protection status")
    print(string.rep("─", 60))

    if not send_command(CMD.GET_VERSION, true) then
        print("  Command failed")
        return false
    end

    local response = wait_for_response(3)

    if not response or #response < 3 then
        print("  Invalid response")
        return false
    end

    local version = response[1]
    local opt1 = response[2]
    local opt2 = response[3]

    print(string.format("\nBootloader Version: 0x%02X (v%d.%d)", 
        version, (version >> 4) & 0x0F, version & 0x0F))
    print(string.format("Option byte 1: 0x%02X", opt1))
    print(string.format("Option byte 2: 0x%02X", opt2))

    return true
end

-- Get chip ID
function cmd_get_id()
    print("\n" .. string.rep("─", 60))
    print("GET ID - Get chip identification")
    print(string.rep("─", 60))

    if not send_command(CMD.GET_ID, true) then
        print("  Command failed")
        return false
    end

    local response = wait_for_response()

    if not response or #response < 3 then
        print("  Invalid response")
        return false
    end

    local n = response[1] + 1
    local chip_id = 0

    for i = 2, n + 1 do
        chip_id = (chip_id << 8) | response[i]
    end

    print(string.format("\nChip ID: 0x%04X", chip_id))

    local chip_name = CHIP_IDS[chip_id]
    if chip_name then
        print(string.format("Device: %s", chip_name))
    else
        print("Device: Unknown")
    end

    return true
end

-- Read memory
function cmd_read_memory()
    print("\n" .. string.rep("─", 60))
    print("READ MEMORY")
    print(string.rep("─", 60))

    local addr_str = core.select_variable("Enter address (hex, e.g., 08000000):")
    if not addr_str or addr_str == "" then
        print("Cancelled.")
        return false
    end

    local address = tonumber(addr_str, 16)
    if not address then
        print("  Invalid address")
        return false
    end

    local len_str = core.select_variable("Enter number of bytes to read (1-256):")
    if not len_str or len_str == "" then
        print("Cancelled.")
        return false
    end

    local length = tonumber(len_str)
    if not length or length < 1 or length > 256 then
        print("  Invalid length (must be 1-256)")
        return false
    end

    if not send_command(CMD.READ_MEMORY, true) then
        print("  Command failed")
        return false
    end

    -- Send address
    local addr_bytes = {
        (address >> 24) & 0xFF,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF
    }

    print(string.format("  Sending address: 0x%08X", address))
    if not send_data(addr_bytes, true) then
        print("  Failed to send address")
        return false
    end

    -- Send number of bytes (N-1)
    local n_minus_1 = length - 1
    print(string.format("  Requesting %d bytes", length))

    local data = pack_bytes(n_minus_1, 0xFF - n_minus_1)
    can_write(config.master_id, 2, data, false)

    -- Wait for ACK
    local ack_response = wait_for_response(1)
    if not ack_response or ack_response[1] ~= ACK then
        print("  No ACK after length")
        return false
    end

    print("  ACK received")

    -- Read data
    print(string.format("  Reading %d bytes...", length))
    local read_data = wait_for_response(length)

    if not read_data or #read_data < length then
        print("  Failed to read data")
        return false
    end

    stats.bytes_read = stats.bytes_read + length

    -- Display data
    print("\nData read:")
    print(string.rep("─", 60))

    for offset = 0, length - 1, 16 do
        local hex_part = {}
        local ascii_part = {}

        for i = 0, 15 do
            if offset + i < length then
                local byte = read_data[offset + i + 1]
                table.insert(hex_part, string.format("%02X", byte))

                if byte >= 0x20 and byte <= 0x7E then
                    table.insert(ascii_part, string.char(byte))
                else
                    table.insert(ascii_part, ".")
                end
            else
                table.insert(hex_part, "  ")
                table.insert(ascii_part, " ")
            end
        end

        print(string.format("%08X: %-48s  %s", 
            address + offset,
            table.concat(hex_part, " "),
            table.concat(ascii_part)))
    end

    return true
end

-- Go - Jump to address
function cmd_go()
    print("\n" .. string.rep("─", 60))
    print("GO - Jump to address")
    print(string.rep("─", 60))

    local addr_str = core.select_variable("Enter address (hex, e.g., 08000000):")
    if not addr_str or addr_str == "" then
        print("Cancelled.")
        return false
    end

    local address = tonumber(addr_str, 16)
    if not address then
        print("  Invalid address")
        return false
    end

    print("WARNING: This will execute code at the specified address!")
    local confirm = core.select_variable("Continue? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.GO, true) then
        print("  Command failed")
        return false
    end

    -- Send address
    local addr_bytes = {
        (address >> 24) & 0xFF,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF
    }

    print(string.format("  Jumping to address: 0x%08X", address))
    if not send_data(addr_bytes, true) then
        print("  Failed to send address")
        return false
    end

    print("  Command executed successfully")
    print("  Device should now be running application code")

    return true
end

-- Write memory
function cmd_write_memory()
    print("\n" .. string.rep("─", 60))
    print("WRITE MEMORY")
    print(string.rep("─", 60))

    local addr_str = core.select_variable("Enter address (hex, e.g., 08000000):")
    if not addr_str or addr_str == "" then
        print("Cancelled.")
        return false
    end

    local address = tonumber(addr_str, 16)
    if not address then
        print("  Invalid address")
        return false
    end

    local data_str = core.select_variable("Enter data (hex, space separated, e.g., 01 02 03 04):")
    if not data_str or data_str == "" then
        print("Cancelled.")
        return false
    end

    -- Parse data bytes
    local data_bytes = {}
    for byte_str in data_str:gmatch("%S+") do
        local byte = tonumber(byte_str, 16)
        if not byte or byte < 0 or byte > 255 then
            print("  Invalid data byte: " .. byte_str)
            return false
        end
        table.insert(data_bytes, byte)
    end

    if #data_bytes == 0 or #data_bytes > 256 then
        print("  Invalid data length (must be 1-256 bytes)")
        return false
    end

    if not send_command(CMD.WRITE_MEMORY, true) then
        print("  Command failed")
        return false
    end

    -- Send address
    local addr_bytes = {
        (address >> 24) & 0xFF,
        (address >> 16) & 0xFF,
        (address >> 8) & 0xFF,
        address & 0xFF
    }

    print(string.format("  Sending address: 0x%08X", address))
    if not send_data(addr_bytes, true) then
        print("  Failed to send address")
        return false
    end

    -- Send data with length
    local write_data = {#data_bytes - 1}
    for _, byte in ipairs(data_bytes) do
        table.insert(write_data, byte)
    end

    print(string.format("  Writing %d bytes", #data_bytes))
    if not send_data(write_data, true) then
        print("  Failed to write data")
        return false
    end

    stats.bytes_written = stats.bytes_written + #data_bytes
    print("  Data written successfully")

    return true
end

-- Erase memory (legacy)
function cmd_erase()
    print("\n" .. string.rep("─", 60))
    print("ERASE - Erase memory (legacy command)")
    print(string.rep("─", 60))
    print("\nOptions:")
    print("  1. Global erase (all sectors)")
    print("  2. Sector erase (specific sectors)")

    local choice = core.select_number("Select option (1-2):")
    if not choice or (choice ~= 1 and choice ~= 2) then
        print("Cancelled.")
        return false
    end

    print("WARNING: This will erase flash memory!")
    local confirm = core.select_variable("Continue? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.ERASE, true) then
        print("  Command failed")
        return false
    end

    if choice == 1 then
        -- Global erase
        print("  Performing global erase...")
        local data = pack_bytes(0xFF, 0x00)
        can_write(config.master_id, 2, data, false)
    else
        -- Sector erase
        local sectors_str = core.select_variable("Enter sector numbers (hex, space separated):")
        if not sectors_str or sectors_str == "" then
            print("Cancelled.")
            return false
        end

        local sectors = {}
        for sector_str in sectors_str:gmatch("%S+") do
            local sector = tonumber(sector_str, 16)
            if sector then
                table.insert(sectors, sector)
            end
        end

        if #sectors == 0 or #sectors > 255 then
            print("  Invalid number of sectors")
            return false
        end

        print(string.format("  Erasing %d sector(s)...", #sectors))

        local erase_data = {#sectors - 1}
        for _, sector in ipairs(sectors) do
            table.insert(erase_data, sector)
        end

        if not send_data(erase_data, true) then
            print("  Erase failed")
            return false
        end
    end

    -- Wait for completion (erase takes longer)
    print("  Waiting for erase completion...")
    delay_ms(500)

    local response = wait_for_response(1)
    if response and response[1] == ACK then
        print("  Erase completed successfully")
        return true
    else
        print("  Erase failed")
        return false
    end
end

-- Extended erase memory
function cmd_extended_erase()
    print("\n" .. string.rep("─", 60))
    print("EXTENDED ERASE - Extended erase memory")
    print(string.rep("─", 60))
    print("\nOptions:")
    print("  1. Global mass erase")
    print("  2. Bank 1 mass erase")
    print("  3. Bank 2 mass erase")
    print("  4. Specific pages erase")

    local choice = core.select_number("Select option (1-4):")
    if not choice or choice < 1 or choice > 4 then
        print("Cancelled.")
        return false
    end

    print("WARNING: This will erase flash memory!")
    local confirm = core.select_variable("Continue? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.EXTENDED_ERASE, true) then
        print("  Command failed")
        return false
    end

    local erase_data = {}

    if choice == 1 then
        -- Global mass erase (0xFFFF)
        print("  Performing global mass erase...")
        erase_data = {0xFF, 0xFF}
    elseif choice == 2 then
        -- Bank 1 mass erase (0xFFFE)
        print("  Performing Bank 1 mass erase...")
        erase_data = {0xFF, 0xFE}
    elseif choice == 3 then
        -- Bank 2 mass erase (0xFFFD)
        print("  Performing Bank 2 mass erase...")
        erase_data = {0xFF, 0xFD}
    else
        -- Specific pages
        local pages_str = core.select_variable("Enter page numbers (hex, space separated):")
        if not pages_str or pages_str == "" then
            print("Cancelled.")
            return false
        end

        local pages = {}
        for page_str in pages_str:gmatch("%S+") do
            local page = tonumber(page_str, 16)
            if page then
                table.insert(pages, page)
            end
        end

        if #pages == 0 then
            print("  No pages specified")
            return false
        end

        print(string.format("  Erasing %d page(s)...", #pages))

        -- N-1 (2 bytes, MSB first)
        local n_minus_1 = #pages - 1
        erase_data = {
            (n_minus_1 >> 8) & 0xFF,
            n_minus_1 & 0xFF
        }

        -- Page numbers (2 bytes each, MSB first)
        for _, page in ipairs(pages) do
            table.insert(erase_data, (page >> 8) & 0xFF)
            table.insert(erase_data, page & 0xFF)
        end
    end

    if not send_data(erase_data, true) then
        print("  Extended erase failed")
        return false
    end

    -- Wait for completion (erase takes longer)
    print("  Waiting for erase completion (this may take several seconds)...")
    delay_ms(2000)

    local response = wait_for_response(1)
    if response and response[1] == ACK then
        print("  Extended erase completed successfully")
        return true
    else
        print("  Extended erase failed")
        return false
    end
end

-- Write protect
function cmd_write_protect()
    print("\n" .. string.rep("─", 60))
    print("WRITE PROTECT - Enable write protection")
    print(string.rep("─", 60))

    local sectors_str = core.select_variable("Enter sector numbers to protect (hex, space separated):")
    if not sectors_str or sectors_str == "" then
        print("Cancelled.")
        return false
    end

    local sectors = {}
    for sector_str in sectors_str:gmatch("%S+") do
        local sector = tonumber(sector_str, 16)
        if sector then
            table.insert(sectors, sector)
        end
    end

    if #sectors == 0 or #sectors > 255 then
        print("  Invalid number of sectors")
        return false
    end

    if not send_command(CMD.WRITE_PROTECT, true) then
        print("  Command failed")
        return false
    end

    print(string.format("  Protecting %d sector(s)...", #sectors))

    local protect_data = {#sectors - 1}
    for _, sector in ipairs(sectors) do
        table.insert(protect_data, sector)
    end

    if not send_data(protect_data, true) then
        print("  Write protect failed")
        return false
    end

    print("  Write protection enabled successfully")
    return true
end

-- Write unprotect
function cmd_write_unprotect()
    print("\n" .. string.rep("─", 60))
    print("WRITE UNPROTECT - Disable write protection")
    print(string.rep("─", 60))

    print("WARNING: This will remove write protection and may cause a system reset!")
    local confirm = core.select_variable("Continue? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.WRITE_UNPROTECT, true) then
        print("  Command failed")
        return false
    end

    -- Wait for completion
    delay_ms(100)

    local response = wait_for_response(1)
    if response and response[1] == ACK then
        print("  Write protection disabled successfully")
        print("  Device may have reset - you may need to reconnect")
        return true
    else
        print("  Write unprotect failed")
        return false
    end
end

-- Readout protect
function cmd_readout_protect()
    print("\n" .. string.rep("─", 60))
    print("READOUT PROTECT - Enable readout protection")
    print(string.rep("─", 60))

    print("WARNING: This will enable readout protection and cause a system reset!")
    print("This may make the device unreadable!")
    local confirm = core.select_variable("Are you sure? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.READOUT_PROTECT, true) then
        print("  Command failed")
        return false
    end

    -- Wait for completion
    delay_ms(100)

    local response = wait_for_response(1)
    if response and response[1] == ACK then
        print("  Readout protection enabled successfully")
        print("  Device has been reset")
        return true
    else
        print("  Readout protect failed")
        return false
    end
end

-- Readout unprotect
function cmd_readout_unprotect()
    print("\n" .. string.rep("─", 60))
    print("READOUT UNPROTECT - Disable readout protection")
    print(string.rep("─", 60))

    print("WARNING: This will disable readout protection, erase all flash, and reset the device!")
    local confirm = core.select_variable("Are you sure? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    if not send_command(CMD.READOUT_UNPROTECT, true) then
        print("  Command failed")
        return false
    end

    -- Wait for completion (this takes longer as it erases flash)
    print("  Waiting for completion (device will erase and reset)...")
    delay_ms(2000)

    local response = wait_for_response(1)
    if response and response[1] == ACK then
        print("  Readout protection disabled successfully")
        print("  All flash has been erased and device has been reset")
        return true
    else
        print("  Readout unprotect failed or device reset occurred")
        return false
    end
end

-- ============================================================
-- Intel HEX File Support
-- ============================================================

-- Parse Intel HEX file
local function parse_intel_hex(filename)
    local file = io.open(filename, "r")
    if not file then
        print("  Error: Could not open file: " .. filename)
        return nil
    end

    local memory_blocks = {}
    local extended_address = 0
    local start_address = nil

    for line_raw in file:lines() do
        -- Remove whitespace
        local line = line_raw:gsub("^%s*(.-)%s*$", "%1")

        -- Skip empty lines
        if line ~= "" then
            -- Verify start code
            if not line:match("^:") then
                print("  Error: Invalid HEX file format (missing start code)")
                file:close()
                return nil
            end

            -- Parse record
            local byte_count = tonumber(line:sub(2, 3), 16)
            local address = tonumber(line:sub(4, 7), 16)
            local record_type = tonumber(line:sub(8, 9), 16)
            local data_start = 10
            local data_end = data_start + (byte_count * 2) - 1

            if record_type == 0x00 then
                -- Data record
                local full_address = extended_address + address
                local data_bytes = {}

                for i = data_start, data_end, 2 do
                    local byte = tonumber(line:sub(i, i + 1), 16)
                    table.insert(data_bytes, byte)
                end

                -- Store in memory blocks
                if not memory_blocks[full_address] then
                    memory_blocks[full_address] = {}
                end

                for _, byte in ipairs(data_bytes) do
                    table.insert(memory_blocks[full_address], byte)
                end

                -- Track lowest address as potential start
                if not start_address or full_address < start_address then
                    start_address = full_address
                end

            elseif record_type == 0x01 then
                -- End of file record
                break

            elseif record_type == 0x04 then
                -- Extended linear address record
                local upper_address = tonumber(line:sub(data_start, data_end), 16)
                extended_address = upper_address << 16

            elseif record_type == 0x05 then
                -- Start linear address record
                start_address = tonumber(line:sub(data_start, data_end), 16)
            end
        end
    end

    file:close()

    -- Convert memory blocks to sorted list
    local sorted_blocks = {}
    for address, data in pairs(memory_blocks) do
        table.insert(sorted_blocks, {address = address, data = data})
    end

    table.sort(sorted_blocks, function(a, b) return a.address < b.address end)

    return sorted_blocks, start_address
end

-- Write hex file to device
function cmd_write_hex_file()
    print("\n" .. string.rep("─", 60))
    print("WRITE HEX FILE")
    print(string.rep("─", 60))

    local hex_path = ""
    if os.getenv("OS") == "Windows_NT" then
        hex_path = os.getenv("USERPROFILE") .. "\\"
    else
        hex_path = "/tmp/"
    end

    local hex_file = core.get_file_by_selection("Select HEX file to write", "hex", hex_path)
    if not hex_file then
        print("Cancelled.")
        return false
    end

    print(string.format("\nParsing file: %s", hex_file))
    local memory_blocks, start_address = parse_intel_hex(hex_file)

    if not memory_blocks then
        print("  Failed to parse HEX file")
        return false
    end

    print(string.format("  Found %d memory block(s)", #memory_blocks))
    if start_address then
        print(string.format("  Start address: 0x%08X", start_address))
    end

    -- Calculate total bytes
    local total_bytes = 0
    for _, block in ipairs(memory_blocks) do
        total_bytes = total_bytes + #block.data
    end
    print(string.format("  Total bytes: %d", total_bytes))

    -- Confirm before writing
    print("\nWARNING: This will write to device flash memory!")
    local confirm = core.select_variable("Continue? [y/N]:")
    if not confirm or confirm:lower() ~= "y" then
        print("Cancelled.")
        return false
    end

    -- Write each block
    local bytes_written = 0
    local start_time = os.clock()

    for block_num, block in ipairs(memory_blocks) do
        local address = block.address
        local data = block.data
        local offset = 0

        print(string.format("\nBlock %d/%d: Address 0x%08X, %d bytes",
            block_num, #memory_blocks, address, #data))

        -- Write in chunks of up to 256 bytes
        while offset < #data do
            local chunk_size = math.min(256, #data - offset)
            local chunk_data = {}

            for i = 1, chunk_size do
                table.insert(chunk_data, data[offset + i])
            end

            -- Send write memory command
            if not send_command(CMD.WRITE_MEMORY, false) then
                print(string.format("  Failed at offset %d", offset))
                return false
            end

            -- Send address
            local current_address = address + offset
            local addr_bytes = {
                (current_address >> 24) & 0xFF,
                (current_address >> 16) & 0xFF,
                (current_address >> 8) & 0xFF,
                current_address & 0xFF
            }

            if not send_data(addr_bytes, false) then
                print(string.format("  Failed to send address at offset %d", offset))
                return false
            end

            -- Send data with length
            local write_data = {chunk_size - 1}
            for _, byte in ipairs(chunk_data) do
                table.insert(write_data, byte)
            end

            if not send_data(write_data, false) then
                print(string.format("  Failed to write data at offset %d", offset))
                return false
            end

            bytes_written = bytes_written + chunk_size
            offset = offset + chunk_size

            -- Progress indicator
            local progress = (bytes_written * 100) / total_bytes
            io.write(string.format("\r  Progress: %d/%d bytes (%.1f%%)", 
                bytes_written, total_bytes, progress))
            io.flush()
        end
    end

    local elapsed_time = os.clock() - start_time
    local bytes_per_sec = bytes_written / elapsed_time

    print(string.format("\n\n  Successfully wrote %d bytes in %.2f seconds (%.0f bytes/sec)",
        bytes_written, elapsed_time, bytes_per_sec))

    stats.bytes_written = stats.bytes_written + bytes_written

    -- Ask if user wants to jump to start address
    if start_address then
        local jump = core.select_variable(
            string.format("\nJump to start address 0x%08X? [y/N]:", start_address))
        if jump and jump:lower() == "y" then
            if not send_command(CMD.GO, true) then
                print("  Failed to send GO command")
                return false
            end

            local addr_bytes = {
                (start_address >> 24) & 0xFF,
                (start_address >> 16) & 0xFF,
                (start_address >> 8) & 0xFF,
                start_address & 0xFF
            }

            if send_data(addr_bytes, true) then
                print("  Device should now be running application")
            end
        end
    end

    return true
end

-- ============================================================
-- Utility Functions
-- ============================================================

-- Display statistics
function display_statistics()
    print("\n" .. string.rep("═", 60))
    print("SESSION STATISTICS")
    print(string.rep("═", 60))
    print(string.format("Commands sent:     %d", stats.commands_sent))
    print(string.format("ACKs received:     %d", stats.acks_received))
    print(string.format("NACKs received:    %d", stats.nacks_received))
    print(string.format("Timeouts:          %d", stats.timeouts))
    print(string.format("Bytes read:        %d", stats.bytes_read))
    print(string.format("Bytes written:     %d", stats.bytes_written))
    print(string.rep("═", 60))
end

-- Display configuration
function display_config()
    print("\n" .. string.rep("─", 60))
    print("CURRENT CONFIGURATION")
    print(string.rep("─", 60))
    print(string.format("Master CAN ID:     0x%03X", config.master_id))
    print(string.format("Slave CAN ID:      0x%03X", config.slave_id))
    print(string.format("Timeout:           %d ms", config.timeout))
    print(string.format("Extended IDs:      %s", config.use_extended_id and "Yes" or "No"))
    print(string.rep("─", 60))
end

-- Configure settings
function configure_settings()
    print("\n" .. string.rep("─", 60))
    print("CONFIGURATION")
    print(string.rep("─", 60))

    local master_str = core.select_variable(
        string.format("Master CAN ID (hex) [current: 0x%03X]:", config.master_id))
    if master_str and master_str ~= "" then
        local val = tonumber(master_str, 16)
        if val then
            config.master_id = val
        end
    end

    local slave_str = core.select_variable(
        string.format("Slave CAN ID (hex) [current: 0x%03X]:", config.slave_id))
    if slave_str and slave_str ~= "" then
        local val = tonumber(slave_str, 16)
        if val then
            config.slave_id = val
        end
    end

    local timeout_str = core.select_variable(
        string.format("Timeout in ms [current: %d]:", config.timeout))
    if timeout_str and timeout_str ~= "" then
        local val = tonumber(timeout_str)
        if val and val > 0 then
            config.timeout = val
        end
    end

    print("\n  Configuration updated")
    display_config()
end

-- ============================================================
-- Main Menu
-- ============================================================

function main_menu()
    while true do
        print("\n" .. string.rep("═", 60))
        print("  AN3154 - STM32 CAN Bootloader Protocol")
        print(string.rep("═", 60))
        print("\nDevice Information Commands:")
        print("  1.  Get - Get version and available commands")
        print("  2.  Get Version - Get version and protection status")
        print("  3.  Get ID - Get chip identification")
        print("\nMemory Operations:")
        print("  4.  Read Memory - Read from device memory")
        print("  5.  Write Memory - Write to device memory")
        print("  6.  Write HEX File - Write Intel HEX file to device")
        print("  7.  Go - Jump to address and execute")
        print("\nErase Operations:")
        print("  8.  Erase - Erase memory (legacy)")
        print("  9.  Extended Erase - Extended erase with more options")
        print("\nProtection Commands:")
        print("  10. Write Protect - Enable write protection")
        print("  11. Write Unprotect - Disable write protection")
        print("  12. Readout Protect - Enable readout protection")
        print("  13. Readout Unprotect - Disable readout protection")
        print("\nUtility:")
        print("  14. Configuration - Configure CAN IDs and timeout")
        print("  15. Show Statistics - Display session statistics")
        print("  16. Clear Statistics - Reset statistics")
        print("  0.  Exit")
        print(string.rep("─", 60))

        local choice = core.select_number("Select option (0-16):")

        if not choice then
            print("\nExiting...")
            break
        end

        can_flush()

        if choice == 1 then
            cmd_get()
        elseif choice == 2 then
            cmd_get_version()
        elseif choice == 3 then
            cmd_get_id()
        elseif choice == 4 then
            cmd_read_memory()
        elseif choice == 5 then
            cmd_write_memory()
        elseif choice == 6 then
            cmd_write_hex_file()
        elseif choice == 7 then
            cmd_go()
        elseif choice == 8 then
            cmd_erase()
        elseif choice == 9 then
            cmd_extended_erase()
        elseif choice == 10 then
            cmd_write_protect()
        elseif choice == 11 then
            cmd_write_unprotect()
        elseif choice == 12 then
            cmd_readout_protect()
        elseif choice == 13 then
            cmd_readout_unprotect()
        elseif choice == 14 then
            configure_settings()
        elseif choice == 15 then
            display_statistics()
        elseif choice == 16 then
            stats = {
                commands_sent = 0,
                acks_received = 0,
                nacks_received = 0,
                timeouts = 0,
                bytes_read = 0,
                bytes_written = 0,
            }
            print("\n  Statistics cleared")
        elseif choice == 0 then
            print("\nExiting...")
            break
        else
            print("\n  Invalid option")
        end
    end
end

-- ============================================================
-- Entry Point
-- ============================================================

print("═══════════════════════════════════════════════════════════")
print("  AN3154 - STM32 CAN Bootloader Protocol Implementation")
print("═══════════════════════════════════════════════════════════")
print("\nThis script implements the ST AN3154 CAN bootloader protocol")
print("for STM32 microcontrollers. It supports firmware updates,")
print("memory operations, and device management over CAN bus.")
print("\nReference: AN3154 Application Note from STMicroelectronics")

display_config()

local start = core.select_variable("\nPress Enter to start or type 'q' to quit:")
if start and start:lower() == "q" then
    print("Exiting.")
    return
end

main_menu()

display_statistics()
print("\n  Session ended")
