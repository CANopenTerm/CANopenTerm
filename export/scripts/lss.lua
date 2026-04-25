--[[ CANopen Layer Setting Services

Author:  Michael Fitzmayer
License: Public domain

--]]

local core = require "core"

-- ============================================================
--  Constants
-- ============================================================

local LSS_TX_ID = 0x7E5  -- Master -> Slave
local LSS_RX_ID = 0x7E4  -- Slave  -> Master

local LSS_CS = {
    SWITCH_MODE_GLOBAL          = 0x04,
    SWITCH_MODE_SEL_VENDOR      = 0x40,
    SWITCH_MODE_SEL_PRODUCT     = 0x41,
    SWITCH_MODE_SEL_REVISION    = 0x42,
    SWITCH_MODE_SEL_SERIAL      = 0x43,
    SWITCH_MODE_SEL_RESP        = 0x44,
    CONFIGURE_NODE_ID           = 0x11,
    CONFIGURE_NODE_ID_RESP      = 0x11,
    CONFIGURE_BIT_TIMING        = 0x13,
    CONFIGURE_BIT_TIMING_RESP   = 0x13,
    STORE_CONFIGURATION         = 0x17,
    STORE_CONFIGURATION_RESP    = 0x17,
    IDENTIFY_SLAVE              = 0x4F,
    IDENTIFY_NON_CFG_SLAVE      = 0x4C,
    INQUIRE_VENDOR_ID           = 0x5A,
    INQUIRE_PRODUCT_CODE        = 0x5B,
    INQUIRE_REVISION_NUMBER     = 0x5C,
    INQUIRE_SERIAL_NUMBER       = 0x5D,
    INQUIRE_NODE_ID             = 0x61,
    INQUIRE_RESP                = 0x5A,  -- same CS range, determined by context
}

-- Bit-timing table index -> rate
local BIT_TIMING_TABLE = {
    [0] = "1 Mbit/s",
    [1] = "800 kbit/s",
    [2] = "500 kbit/s",
    [3] = "250 kbit/s",
    [4] = "125 kbit/s",
    [5] = "100 kbit/s",
    [6] = "50 kbit/s",
    [7] = "20 kbit/s",
    [8] = "10 kbit/s",
    [9] = "Auto detection",
}

local RESPONSE_TIMEOUT_MS = 500

-- ============================================================
--  Helper utilities
-- ============================================================

-- Pack a byte-table (index 1 = first CAN byte) into a uint64
-- suitable for can_write().  First byte occupies the MSB.
local function pack_frame(t)
    local data = 0
    for i = 1, math.min(#t, 8) do
        data = data | (t[i] << ((8 - i) * 8))
    end
    return data
end

-- Extract byte n (1-based, first byte = MSB) from a packed uint64.
local function byte_at(raw, n)
    return (raw >> ((8 - n) * 8)) & 0xFF
end

-- Build an 8-byte LSS frame: CS in byte 1, remaining bytes as given.
local function lss_frame(cs, b2, b3, b4, b5, b6, b7, b8)
    return pack_frame({
        cs,
        b2 or 0x00, b3 or 0x00, b4 or 0x00,
        b5 or 0x00, b6 or 0x00, b7 or 0x00, b8 or 0x00
    })
end

-- Pack a 32-bit value little-endian into bytes 2-5 of an LSS frame.
local function lss_frame_u32(cs, value)
    local b2 = value        & 0xFF
    local b3 = (value >> 8)  & 0xFF
    local b4 = (value >> 16) & 0xFF
    local b5 = (value >> 24) & 0xFF
    return lss_frame(cs, b2, b3, b4, b5)
end

-- Wait up to RESPONSE_TIMEOUT_MS for an LSS response with the given CS.
-- Returns data (uint64) or nil on timeout.
local function wait_for_response(expected_cs)
    local deadline = os.clock() + RESPONSE_TIMEOUT_MS / 1000
    while os.clock() < deadline do
        local id, length, data = can_read()
        if id == LSS_RX_ID and length == 8 then
            local cs = byte_at(data, 1)
            if cs == expected_cs then
                return data
            end
        end
    end
    return nil
end

-- ============================================================
--  LSS services
-- ============================================================

-- Switch Mode Global: 0 = Waiting, 1 = Configuration
local function lss_switch_mode_global(mode)
    local frame = lss_frame(LSS_CS.SWITCH_MODE_GLOBAL, mode)
    can_write(LSS_TX_ID, 8, frame)
    -- No response expected for global switch
end

-- Switch Mode Selective using 4-word identity (vendor, product, revision, serial).
-- Returns true if slave acknowledged.
local function lss_switch_mode_selective(vendor_id, product_code, revision, serial)
    can_write(LSS_TX_ID, 8, lss_frame_u32(LSS_CS.SWITCH_MODE_SEL_VENDOR,   vendor_id))
    can_write(LSS_TX_ID, 8, lss_frame_u32(LSS_CS.SWITCH_MODE_SEL_PRODUCT,  product_code))
    can_write(LSS_TX_ID, 8, lss_frame_u32(LSS_CS.SWITCH_MODE_SEL_REVISION, revision))
    can_write(LSS_TX_ID, 8, lss_frame_u32(LSS_CS.SWITCH_MODE_SEL_SERIAL,   serial))
    local resp = wait_for_response(LSS_CS.SWITCH_MODE_SEL_RESP)
    return resp ~= nil
end

-- Configure Node-ID. Returns error_code (0 = OK) or nil on timeout.
local function lss_configure_node_id(node_id)
    local frame = lss_frame(LSS_CS.CONFIGURE_NODE_ID, node_id)
    can_write(LSS_TX_ID, 8, frame)
    local resp = wait_for_response(LSS_CS.CONFIGURE_NODE_ID_RESP)
    if resp then
        return byte_at(resp, 2)  -- error code
    end
    return nil
end

-- Configure Bit Timing. table_selector is typically 0. Returns error_code or nil.
local function lss_configure_bit_timing(table_selector, bit_timing_index)
    local frame = lss_frame(LSS_CS.CONFIGURE_BIT_TIMING, table_selector, bit_timing_index)
    can_write(LSS_TX_ID, 8, frame)
    local resp = wait_for_response(LSS_CS.CONFIGURE_BIT_TIMING_RESP)
    if resp then
        return byte_at(resp, 2)
    end
    return nil
end

-- Store Configuration. Returns error_code or nil.
local function lss_store_configuration()
    local frame = lss_frame(LSS_CS.STORE_CONFIGURATION)
    can_write(LSS_TX_ID, 8, frame)
    local resp = wait_for_response(LSS_CS.STORE_CONFIGURATION_RESP)
    if resp then
        return byte_at(resp, 2)
    end
    return nil
end

-- Identify Slave (broadcast). Collects responses for RESPONSE_TIMEOUT_MS ms.
-- Returns list of raw response data frames.
local function lss_identify_slaves()
    can_flush()
    local frame = lss_frame(LSS_CS.IDENTIFY_SLAVE)
    can_write(LSS_TX_ID, 8, frame)

    local deadline = os.clock() + RESPONSE_TIMEOUT_MS / 1000
    local responses = {}
    while os.clock() < deadline do
        local id, length, data = can_read()
        if id == LSS_RX_ID and length == 8 then
            local cs = byte_at(data, 1)
            if cs == LSS_CS.IDENTIFY_SLAVE then
                table.insert(responses, data)
            end
        end
    end
    return responses
end

-- Identify Non-Configured Slaves (no node-ID assigned). Returns list of responses.
local function lss_identify_non_configured_slaves()
    can_flush()
    local frame = lss_frame(LSS_CS.IDENTIFY_NON_CFG_SLAVE)
    can_write(LSS_TX_ID, 8, frame)

    local deadline = os.clock() + RESPONSE_TIMEOUT_MS / 1000
    local responses = {}
    while os.clock() < deadline do
        local id, length, data = can_read()
        if id == LSS_RX_ID and length == 8 then
            local cs = byte_at(data, 1)
            if cs == LSS_CS.IDENTIFY_SLAVE then
                table.insert(responses, data)
            end
        end
    end
    return responses
end

-- Inquire a 32-bit identity field (vendor/product/revision/serial) or node-ID.
-- Returns value (number) or nil on timeout.
local function lss_inquire(cs_request, cs_response)
    local frame = lss_frame(cs_request)
    can_write(LSS_TX_ID, 8, frame)
    local resp = wait_for_response(cs_response)
    if resp then
        local b2 = byte_at(resp, 2)
        local b3 = byte_at(resp, 3)
        local b4 = byte_at(resp, 4)
        local b5 = byte_at(resp, 5)
        return b2 | (b3 << 8) | (b4 << 16) | (b5 << 24)
    end
    return nil
end

-- ============================================================
--  Menu actions
-- ============================================================

local function action_switch_mode_global()
    print("\nSwitch Mode Global")
    print("  0 = Waiting (Normal operation)")
    print("  1 = Configuration")
    local mode = core.select_number("Select mode:")
    if mode == nil then return end
    if mode ~= 0 and mode ~= 1 then
        print("Invalid mode. Use 0 or 1.")
        return
    end
    lss_switch_mode_global(mode)
    print(string.format("All nodes switched to %s mode.", mode == 1 and "Configuration" or "Waiting"))
end

local function action_switch_mode_selective()
    print("\nSwitch Mode Selective (by Identity Object)")
    local vendor = core.select_variable("Vendor ID (hex):")
    if vendor == nil then return end
    local product = core.select_variable("Product Code (hex):")
    if product == nil then return end
    local revision = core.select_variable("Revision Number (hex):")
    if revision == nil then return end
    local serial = core.select_variable("Serial Number (hex):")
    if serial == nil then return end

    local vid = tonumber(vendor, 16)
    local pid = tonumber(product, 16)
    local rev = tonumber(revision, 16)
    local sn  = tonumber(serial, 16)

    if not (vid and pid and rev and sn) then
        print("Invalid input.")
        return
    end

    print(string.format("Selecting node: Vendor=0x%08X  Product=0x%08X  Rev=0x%08X  SN=0x%08X",
        vid, pid, rev, sn))

    if lss_switch_mode_selective(vid, pid, rev, sn) then
        print("Node acknowledged: now in Configuration mode.")
    else
        print("No response received (timeout).")
    end
end

local function action_configure_node_id()
    print("\nConfigure Node-ID  (node must be in Configuration mode)")
    local inp = core.select_variable("New Node-ID (hex, 1-7F):")
    if inp == nil then return end
    local node_id = tonumber(inp, 16)
    if not node_id or node_id < 1 or node_id > 0x7F then
        print("Invalid Node-ID. Must be 0x01..0x7F.")
        return
    end

    local err = lss_configure_node_id(node_id)
    if err == nil then
        print("No response received (timeout).")
    elseif err == 0 then
        print(string.format("Node-ID 0x%02X configured successfully.", node_id))
    elseif err == 1 then
        print("Error: Node-ID out of range.")
    elseif err == 0xFF then
        print("Error: Implementation-specific error.")
    else
        print(string.format("Error code: 0x%02X", err))
    end
end

local function action_configure_bit_timing()
    print("\nConfigure Bit Timing  (node must be in Configuration mode)")
    print("Bit timing table (table selector 0):")
    for i = 0, 9 do
        print(string.format("  %d = %s", i, BIT_TIMING_TABLE[i]))
    end
    local idx = core.select_number("Select bit timing index (0-9):")
    if idx == nil then return end
    if idx < 0 or idx > 9 then
        print("Invalid index.")
        return
    end

    local err = lss_configure_bit_timing(0, idx)
    if err == nil then
        print("No response received (timeout).")
    elseif err == 0 then
        print(string.format("Bit timing set to %s.", BIT_TIMING_TABLE[idx]))
    elseif err == 1 then
        print("Error: Bit timing not supported.")
    elseif err == 0xFF then
        print("Error: Implementation-specific error.")
    else
        print(string.format("Error code: 0x%02X", err))
    end
end

local function action_store_configuration()
    print("\nStore Configuration  (node must be in Configuration mode)")
    local err = lss_store_configuration()
    if err == nil then
        print("No response received (timeout).")
    elseif err == 0 then
        print("Configuration stored successfully.")
    elseif err == 1 then
        print("Error: Store not supported.")
    elseif err == 2 then
        print("Error: Storage media access error.")
    elseif err == 0xFF then
        print("Error: Implementation-specific error.")
    else
        print(string.format("Error code: 0x%02X", err))
    end
end

local function action_identify_slaves()
    print("\nIdentify Slaves (broadcast)...")
    local responses = lss_identify_slaves()
    if #responses == 0 then
        print("No slaves responded.")
    else
        print(string.format("%d slave(s) responded.", #responses))
    end
end

local function action_identify_non_configured()
    print("\nIdentify Non-Configured Slaves...")
    local responses = lss_identify_non_configured_slaves()
    if #responses == 0 then
        print("No non-configured slaves found.")
    else
        print(string.format("%d non-configured slave(s) responded.", #responses))
    end
end

local function action_inquire_identity()
    print("\nInquire Identity  (node must be in Configuration mode)")
    local vendor   = lss_inquire(LSS_CS.INQUIRE_VENDOR_ID,       0x5A)
    local product  = lss_inquire(LSS_CS.INQUIRE_PRODUCT_CODE,    0x5B)
    local revision = lss_inquire(LSS_CS.INQUIRE_REVISION_NUMBER, 0x5C)
    local serial   = lss_inquire(LSS_CS.INQUIRE_SERIAL_NUMBER,   0x5D)
    local node_id  = lss_inquire(LSS_CS.INQUIRE_NODE_ID,         0x61)

    print(string.rep("─", 50))
    print(string.format("  Vendor ID      : %s", vendor   and string.format("0x%08X", vendor)   or "timeout"))
    print(string.format("  Product Code   : %s", product  and string.format("0x%08X", product)  or "timeout"))
    print(string.format("  Revision Number: %s", revision and string.format("0x%08X", revision) or "timeout"))
    print(string.format("  Serial Number  : %s", serial   and string.format("0x%08X", serial)   or "timeout"))
    print(string.format("  Node-ID        : %s", node_id  and string.format("0x%02X",  node_id)  or "timeout"))
    print(string.rep("─", 50))
end

-- ============================================================
--  Main menu
-- ============================================================

print("═══════════════════════════════════════════════════════════")
print("  CANopen Layer Setting Services (LSS)")
print("═══════════════════════════════════════════════════════════")
print(string.format("  LSS TX: 0x%03X  (Master -> Slave)", LSS_TX_ID))
print(string.format("  LSS RX: 0x%03X  (Slave  -> Master)", LSS_RX_ID))

while true do
    print("\nActions:")
    print("  1. Switch Mode Global")
    print("  2. Switch Mode Selective")
    print("  3. Configure Node-ID")
    print("  4. Configure Bit Timing")
    print("  5. Store Configuration")
    print("  6. Identify Slaves")
    print("  7. Identify Non-Configured Slaves")
    print("  8. Inquire Identity / Node-ID")
    print("  0. Exit")

    local choice = core.select_number("Select action:")
    if choice == nil or choice == 0 then
        print("Exiting.")
        break
    elseif choice == 1 then
        action_switch_mode_global()
    elseif choice == 2 then
        action_switch_mode_selective()
    elseif choice == 3 then
        action_configure_node_id()
    elseif choice == 4 then
        action_configure_bit_timing()
    elseif choice == 5 then
        action_store_configuration()
    elseif choice == 6 then
        action_identify_slaves()
    elseif choice == 7 then
        action_identify_non_configured()
    elseif choice == 8 then
        action_inquire_identity()
    else
        print("Invalid choice.")
    end
end
