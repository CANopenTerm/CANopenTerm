--[[

Title:   ISOBUS (ISO 11783) utilities
Author:  Michael Fitzmayer
License: Public domain

--]]

-- Clear any cached version of this module to ensure proper loading
package.loaded["isobus"] = nil

-- Extract PGN from 29-bit extended CAN ID
-- ISO 11783 ID format: [P P PS DA SA] where P=Priority, PS=PS, DA=Dest Addr, SA=Source Addr
-- PGN = (ID >> 8) & 0x3FFFF
local function extract_pgn(id)
    return (id >> 8) & 0x3FFFF
end

-- Extract source address from CAN ID
local function extract_source_addr(id)
    return id & 0xFF
end

-- Extract destination address from CAN ID
local function extract_dest_addr(id)
    return (id >> 8) & 0xFF
end

-- PGN Database with descriptions and decoders
local pgn_database = {
    -- Electronic Engine Controller 1 (EEC1)
    [0x0F004] = {name = "EEC1", desc = "Electronic Engine Controller 1 - Engine load, throttle, cruise control"},
    -- Electronic Engine Controller 2 (EEC2)
    [0x0F003] = {name = "EEC2", desc = "Electronic Engine Controller 2 - Timing/derate and enable RPM"},
    -- Engine Temperature 1 (ET1)
    [0x0FFEA] = {name = "ET1", desc = "Engine Temperature 1 - Coolant temperature, oil temperature"},
    -- Engine Speed/Load 1 (ERC1)
    [0x0F004] = {name = "ERC1", desc = "Electronic Retarder Controller 1 - Retarder enable"},
    -- Transmission Controller 1 (TC1)
    [0x0F005] = {name = "TC1", desc = "Transmission Controller 1 - Transmission state"},
    -- Engine Hours (HOURS)
    [0x0FEE0] = {name = "HOURS", desc = "Engine Operating Hours"},
    -- Electronic Brake Controller (EBC1)
    [0x0F001] = {name = "EBC1", desc = "Electronic Brake Controller 1 - Brake pedal pressure, brake switch"},
    -- Cruise Control Electronic Control Unit (CCVS)
    [0x0FEF1] = {name = "CCVS", desc = "Cruise Control/Vehicle Speed - Vehicle speed, cruise control status"},
    -- Tractor ECU 1 (TE1)
    [0x0FEE5] = {name = "TE1", desc = "Tractor ECU 1 - Tractor status information"},
    -- Tractor ECU (TE)
    [0x0FEE6] = {name = "TE", desc = "Tractor ECU - Additional tractor status"},
    -- Hitch Control (HIC)
    [0x0FE8A] = {name = "HIC", desc = "Hitch Control - Hitch position feedback"},
    -- Implement Power Take-Off 1 (IPC1)
    [0x0FEE7] = {name = "IPC1", desc = "Implement Power Take-Off - PTO shaft speed"},
    -- Pump Flow Control (PFC)
    [0x0FEE8] = {name = "PFC", desc = "Pump Flow Control - Hydraulic pump flow"},
    -- Lighting Data (LD)
    [0x0FBE6] = {name = "LD", desc = "Lighting Data - Light switch status"},
    -- Vehicle Electrical Power 1 (VEP1)
    [0x0FFDF] = {name = "VEP1", desc = "Vehicle Electrical Power 1 - Battery voltage, alternator output"},
    -- Alternator Output 1 (AO)
    [0x0FDE4] = {name = "AO", desc = "Alternator Output 1 - Alternator charge current"},
    -- Fuel System (FS)
    [0x0FFEE] = {name = "FS", desc = "Fuel System - Fuel level, fuel consumption"},
    -- DC Battery Monitor (DCBM)
    [0x0FFEC] = {name = "DCBM", desc = "DC Battery Monitor - Battery state"},
    -- Ground Based Speed (GBS)
    [0x0FDE8] = {name = "GBS", desc = "Ground Based Speed - Vehicle ground speed"},
    -- Implement Position Control 1 (IPC)
    [0x0FD36] = {name = "IPC", desc = "Implement Position Control 1"},
    -- Cabin Climate Control 1 (CCC1)
    [0x0FE07] = {name = "CCC1", desc = "Cabin Climate Control 1 - Temperature, fan speed"},
    -- High Resolution Vehicle Speed (HRVS)
    [0x0FE48] = {name = "HRVS", desc = "High Resolution Vehicle Speed"},
    -- Front Axle Information (FAI)
    [0x0FE8F] = {name = "FAI", desc = "Front Axle Information"},
    -- Rear Axle Information (RAI)
    [0x0FEA0] = {name = "RAI", desc = "Rear Axle Information"},
    -- Implement Electronic Control Unit 1 (IECU1)
    [0x0FEF8] = {name = "IECU1", desc = "Implement Electronic Control Unit 1 - Implement status"},
    -- Diag Data (DD)
    [0x0FECA] = {name = "DD", desc = "Diagnostic Data"},
    -- Identification Information (II)
    [0x0FEEC] = {name = "II", desc = "Identification Information - Make, model, serial number"},
    -- Proprietary A (PA)
    [0x0EF00] = {name = "PA", desc = "Proprietary A Message"},
    -- Proprietary B (PB)
    [0x0EF01] = {name = "PB", desc = "Proprietary B Message"},
    -- Proprietary C (PC)
    [0x0EF02] = {name = "PC", desc = "Proprietary C Message"},
    -- Proprietary D (PD)
    [0x0EF03] = {name = "PD", desc = "Proprietary D Message"},
    -- Request/Acknowledge (ACK/NACK)
    [0x0EE00] = {name = "ACK", desc = "Request/Acknowledge Message"},
    -- Transport Protocol Data Transfer (TP.DT)
    [0x0EB00] = {name = "TP.DT", desc = "Transport Protocol Data Transfer"},
    -- Transport Protocol Connection Management (TP.CM)
    [0x0EC00] = {name = "TP.CM", desc = "Transport Protocol Connection Management"},
}

-- Create reverse lookup from PGN name to PGN value
local pgn_name_to_value = {}
for pgn_value, pgn_info in pairs(pgn_database) do
    pgn_name_to_value[pgn_info.name] = pgn_value
end

-- Decode common ISOBUS message data
local function decode_message_bytes(data, length)
    local bytes = {}
    for i = length - 1, 0, -1 do
        local byte = (data >> (8 * i)) & 0xFF
        table.insert(bytes, string.format("%02X", byte))
    end
    return table.concat(bytes, " ")
end

-- Main parse function
local function parse(id, length, data)
    -- ISOBUS uses 29-bit extended CAN IDs
    -- Check if this looks like an ISOBUS ID (has extended ID characteristic)
    if not (id & 0x80000000 > 0) and id > 0x7FF then
        -- Could be ISOBUS if ID is large enough for extended format
        local pgn = extract_pgn(id)
        local source_addr = extract_source_addr(id)
        local dest_addr = extract_dest_addr(id)

        -- Look up PGN in database
        local pgn_info = pgn_database[pgn]

        if pgn_info then
            local desc = string.format("ISOBUS %s [PGN: %05X] - %s (SA: %02X, DA: %02X)",
                pgn_info.name, pgn, pgn_info.desc, source_addr, dest_addr)
            return desc
        else
            -- Unknown PGN but still ISOBUS format
            return string.format("ISOBUS [PGN: %05X] - Unknown message (SA: %02X, DA: %02X)",
                pgn, source_addr, dest_addr)
        end
    end

    return nil
end

-- Export functions
return {
    parse = parse,
    extract_pgn = extract_pgn,
    extract_source_addr = extract_source_addr,
    extract_dest_addr = extract_dest_addr,
    pgn_database = pgn_database,
    pgn_name_to_value = pgn_name_to_value
}
