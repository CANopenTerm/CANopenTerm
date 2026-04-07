--[[ Unified Diagnostic Services

Author:  Michael Fitzmayer
License: Public domain
Comment: Read UDS data according to ISO 14229-1.
         This script currently only works with 11-bit CAN IDs and does not support multi-frame messages.

--]]

local core = require "core"

-- UDS Service IDs (ISO 14229-1, Table 2)
local SID = {
    DIAGNOSTIC_SESSION_CONTROL = 0x10,
    ECU_RESET                  = 0x11,
    READ_DTC_INFORMATION       = 0x19,
    READ_DATA_BY_IDENTIFIER    = 0x22,
    TESTER_PRESENT             = 0x3E,
    NEGATIVE_RESPONSE          = 0x7F,
}

local POSITIVE_RESPONSE_OFFSET = 0x40

-- Negative Response Codes (ISO 14229-1, Table A.1)
local NRC_DESC = {
    [0x10] = "generalReject",
    [0x11] = "serviceNotSupported",
    [0x12] = "subFunctionNotSupported",
    [0x13] = "incorrectMessageLengthOrInvalidFormat",
    [0x14] = "responseTooLong",
    [0x21] = "busyRepeatRequest",
    [0x22] = "conditionsNotCorrect",
    [0x24] = "requestSequenceError",
    [0x25] = "noResponseFromSubnetComponent",
    [0x26] = "failurePreventsExecutionOfRequestedAction",
    [0x31] = "requestOutOfRange",
    [0x33] = "securityAccessDenied",
    [0x35] = "invalidKey",
    [0x36] = "exceededNumberOfAttempts",
    [0x37] = "requiredTimeDelayNotExpired",
    [0x70] = "uploadDownloadNotAccepted",
    [0x71] = "transferDataSuspended",
    [0x72] = "generalProgrammingFailure",
    [0x73] = "wrongBlockSequenceCounter",
    [0x78] = "requestCorrectlyReceivedResponsePending",
    [0x7E] = "subFunctionNotSupportedInActiveSession",
    [0x7F] = "serviceNotSupportedInActiveSession",
}

-- Standard Data Identifiers (ISO 14229-1, Table C.1)
local DID_INFO = {
    [0xF180] = "Boot Software Identification",
    [0xF181] = "Application Software Identification",
    [0xF182] = "Application Data Identification",
    [0xF183] = "Boot Software Fingerprint",
    [0xF184] = "Application Software Fingerprint",
    [0xF185] = "Application Data Fingerprint",
    [0xF186] = "Active Diagnostic Session",
    [0xF187] = "VM Spare Part Number",
    [0xF188] = "VM ECU Software Number",
    [0xF189] = "VM ECU Software Version Number",
    [0xF18A] = "System Supplier Identifier",
    [0xF18B] = "ECU Manufacturing Date",
    [0xF18C] = "ECU Serial Number",
    [0xF18D] = "Supported Functional Units",
    [0xF18E] = "VM Kit Assembly Part Number",
    [0xF190] = "Vehicle Identification Number (VIN)",
    [0xF191] = "VM ECU Hardware Number",
    [0xF192] = "System Supplier ECU Hardware Number",
    [0xF193] = "System Supplier ECU Hardware Version",
    [0xF194] = "System Supplier ECU Software Number",
    [0xF195] = "System Supplier ECU Software Version",
    [0xF196] = "Exhaust Regulation / Type Approval Number",
    [0xF197] = "System Name or Engine Type",
    [0xF198] = "Repair Shop Code / Tester Serial Number",
    [0xF199] = "Programming Date",
    [0xF19A] = "Calibration Repair Shop Code",
    [0xF19B] = "Calibration Date",
    [0xF19C] = "Calibration Equipment Software Number",
    [0xF19D] = "ECU Installation Date",
    [0xF19E] = "ODX File",
    [0xF19F] = "Entity",
}

-- DTC status bit descriptions (ISO 14229-1, Table D.2)
local DTC_STATUS = {
    [0] = "testFailed",
    [1] = "testFailedThisOperationCycle",
    [2] = "pendingDTC",
    [3] = "confirmedDTC",
    [4] = "testNotCompletedSinceLastClear",
    [5] = "testFailedSinceLastClear",
    [6] = "testNotCompletedThisOperationCycle",
    [7] = "warningIndicatorRequested",
}

-- Diagnostic session descriptions (ISO 14229-1, Table 40)
local SESSION_DESC = {
    [0x01] = "defaultSession",
    [0x02] = "programmingSession",
    [0x03] = "extendedDiagnosticSession",
    [0x04] = "safetySystemDiagnosticSession",
}

-- DTC format identifiers (ISO 14229-1, Table 218)
local DTC_FORMAT_DESC = {
    [0x00] = "ISO 15031-6",
    [0x01] = "ISO 14229-1",
    [0x02] = "SAE J2012-DA",
    [0x03] = "ISO 11992-4",
}

-- NRC values treated as silent (DID simply not available on this ECU)
local SILENT_NRC = {
    ["timeout"]                             = true,
    ["NRC serviceNotSupported"]             = true,
    ["NRC requestOutOfRange"]               = true,
    ["NRC subFunctionNotSupported"]         = true,
    ["NRC serviceNotSupportedInActiveSession"] = true,
}

-- Configuration (set during setup)
local request_id = 0x7DF
local response_id = 0x7E8
local timeout_ms  = 1000

-- Get the n-th byte (0-based) from raw can_read() data.
-- can_read() returns a uint64 via os_memcpy on little-endian hardware,
-- so byte 0 of the CAN frame sits at the least-significant byte position.
local function byte_at(raw, n)
    return (raw >> (n * 8)) & 0xFF
end

-- Pack a byte-table into a uint64 suitable for can_write().
-- can_write() maps bit 56..63 -> data[0], bit 48..55 -> data[1], ...
local function pack_frame(t)
    local data = 0
    for i = 1, math.min(#t, 8) do
        data = data | (t[i] << ((8 - i) * 8))
    end
    return data
end

-- Return true when bytes [from, from+count-1] (0-based) are all printable ASCII.
local function is_printable(raw, from, count)
    for i = from, from + count - 1 do
        local b = byte_at(raw, i)
        if b < 0x20 or b > 0x7E then return false end
    end
    return true
end

-- Render bytes as a string; non-printable bytes shown as [XX].
local function raw_to_str(raw, from, count)
    local out = ""
    for i = from, from + count - 1 do
        local b = byte_at(raw, i)
        if b >= 0x20 and b <= 0x7E then
            out = out .. string.char(b)
        else
            out = out .. string.format("[%02X]", b)
        end
    end
    return out
end

-- Render bytes as a space-separated hex string.
local function raw_to_hex(raw, from, count)
    local parts = {}
    for i = from, from + count - 1 do
        table.insert(parts, string.format("%02X", byte_at(raw, i)))
    end
    return table.concat(parts, " ")
end

-- Decode the data payload of a ReadDataByIdentifier response into a string.
-- Bytes 0-3 are PCI/SID/DID_H/DID_L; actual data starts at byte 4.
-- Single-frame UDS responses carry at most 4 data bytes after the 3-byte header.
local function decode_did_value(did, raw, data_len)
    if data_len <= 0 then return "(no data)" end
    local from  = 4
    local count = math.min(data_len, 4)

    if did == 0xF186 then
        local s = byte_at(raw, from)
        return SESSION_DESC[s] or string.format("0x%02X", s)
    elseif did == 0xF18B or did == 0xF199 or did == 0xF19B or did == 0xF19D then
        -- Date: Year(2 bytes BCD), Month(1 byte BCD), Day(1 byte BCD)
        if data_len >= 4 then
            return string.format("%02X%02X-%02X-%02X",
                byte_at(raw, from),   byte_at(raw, from + 1),
                byte_at(raw, from + 2), byte_at(raw, from + 3))
        end
        return raw_to_hex(raw, from, count)
    elseif is_printable(raw, from, count) then
        return raw_to_str(raw, from, count)
    else
        return raw_to_hex(raw, from, count)
    end
end

-- Format a 3-byte UDS DTC as an alphanumeric code (e.g. P0300).
-- b3 is the Failure Type Byte and is appended only when non-zero.
local function format_dtc(b1, b2, b3)
    local prefix = ({"P", "C", "B", "U"})[((b1 >> 6) & 0x03) + 1]
    local d1     = (b1 >> 4) & 0x03
    local d2     =  b1       & 0x0F
    if b3 ~= 0x00 then
        return string.format("%s%d%X%02X FT:%02X", prefix, d1, d2, b2, b3)
    end
    return string.format("%s%d%X%02X", prefix, d1, d2, b2)
end

-- Wait for a positive (or negative) UDS response addressed to response_id.
-- Handles NRC 0x78 (response pending) by resetting the deadline.
local function uds_receive(expected_sid)
    local deadline = os.clock() + timeout_ms / 1000
    while os.clock() < deadline or not key_is_hit() do
        local id, _, raw = can_read()
        if id == response_id and raw ~= nil then
            local pci      = byte_at(raw, 0)
            local pci_type = (pci >> 4) & 0x0F
            local sf_len   =  pci       & 0x0F

            if pci_type ~= 0 then
                return nil, 0, "multi-frame"
            end

            local sid = byte_at(raw, 1)
            if sid == SID.NEGATIVE_RESPONSE then
                if byte_at(raw, 2) == expected_sid then
                    local nrc = byte_at(raw, 3)
                    if nrc == 0x78 then
                        deadline = os.clock() + timeout_ms / 1000
                    else
                        local desc = NRC_DESC[nrc] or string.format("0x%02X", nrc)
                        return nil, 0, "NRC " .. desc
                    end
                end
            elseif sid == (expected_sid + POSITIVE_RESPONSE_OFFSET) then
                return raw, sf_len, nil
            end
        end
    end
    return nil, 0, "timeout"
end

-- Build and send a single-frame UDS request.
local function uds_send(sid, payload)
    local frame = {1 + #payload, sid}
    for _, b in ipairs(payload) do table.insert(frame, b) end
    while #frame < 8 do table.insert(frame, 0x00) end
    can_write(request_id, 8, pack_frame(frame))
end

-- Send a request and wait for the matching response.
local function uds_request(sid, payload)
    uds_send(sid, payload)
    return uds_receive(sid)
end

-- DiagnosticSessionControl (0x10)
local function session_control(session_type)
    local raw, _, err = uds_request(SID.DIAGNOSTIC_SESSION_CONTROL, {session_type})
    return raw ~= nil, err
end

-- TesterPresent (0x3E) with suppress-positive-response bit set (0x80)
local function tester_present()
    uds_send(SID.TESTER_PRESENT, {0x80})
end

-- ReadDataByIdentifier (0x22)
-- Returns: raw, data_len, err
-- Data starts at byte offset 4 in raw; data_len is the number of payload bytes.
local function read_did(did)
    local raw, sf_len, err = uds_request(
        SID.READ_DATA_BY_IDENTIFIER,
        {(did >> 8) & 0xFF, did & 0xFF}
    )
    if not raw then return nil, 0, err end
    return raw, sf_len - 3, nil   -- subtract SID(1) + DID_H(1) + DID_L(1)
end

-- ReadDTCInformation (0x19)
local function read_dtc(sub_fn, status_mask)
    local payload = {sub_fn}
    if status_mask then table.insert(payload, status_mask) end
    return uds_request(SID.READ_DTC_INFORMATION, payload)
end

-- ============================================================
--  Setup
-- ============================================================

local inp = core.select_variable("ECU request CAN-ID in hex (default: 7DF)")
if inp == nil then print("Exiting.") return end
if inp ~= "" then
    local val = tonumber(inp, 16)
    if val then
        request_id = val
        if request_id >= 0x7E0 and request_id <= 0x7E7 then
            response_id = request_id + 8
        end
    end
end

inp = core.select_variable(
    string.format("ECU response CAN-ID in hex (default: %03X)", response_id))
if inp == nil then print("Exiting.") return end
if inp ~= "" then
    local val = tonumber(inp, 16)
    if val then response_id = val end
end

inp = core.select_variable(
    string.format("Response timeout in ms (default: %d)", timeout_ms))
if inp == nil then print("Exiting.") return end
if inp ~= "" then
    local val = tonumber(inp)
    if val and val > 0 then timeout_ms = val end
end

can_flush()
print(string.format("UDS Diagnostic  |  Req: 0x%03X  Rsp: 0x%03X",
    request_id, response_id))

io.write("Opening default diagnostic session (0x10, sub: 0x01) ... ")
io.flush()
local ok, session_err = session_control(0x01)
if ok then
    print("OK")
else
    print(string.format("FAILED (%s) - continuing", session_err or "?"))
end

-- ============================================================
--  Section 1: ECU Identification (ReadDataByIdentifier 0x22)
-- ============================================================

print("")
print("── ECU Identification (0x22) " .. string.rep("─", 47))
print(string.format("  %-6s  %-44s  %s", "DID", "Identifier", "Value"))
print(string.rep("─", 76))

local found = 0
for did = 0xF180, 0xF19F do
    if key_is_hit() then
        print("  (interrupted by user)")
        break
    end

    if did ~= 0xF18F then   -- 0xF18F is ISOSAEReserved
        local raw, data_len, err = read_did(did)
        if raw then
            found = found + 1
            local name  = DID_INFO[did] or string.format("DID_0x%04X", did)
            local value = decode_did_value(did, raw, data_len)
            print(string.format("  0x%04X  %-44s  %s", did, name, value))
        elseif err == "multi-frame" then
            found = found + 1
            local name = DID_INFO[did] or string.format("DID_0x%04X", did)
            print(string.format("  0x%04X  %-44s  [supported, multi-frame]", did, name))
        elseif not SILENT_NRC[err] then
            local name = DID_INFO[did] or string.format("DID_0x%04X", did)
            print(string.format("  0x%04X  %-44s  [%s]", did, name, err or "?"))
        end
    end
end

if found == 0 then
    print("  No standard identification DIDs supported by this ECU.")
end

-- ============================================================
--  Section 2: DTC Information (ReadDTCInformation 0x19)
-- ============================================================

print("")
print("── Diagnostic Trouble Codes (0x19) " .. string.rep("─", 41))

-- Sub-function 0x01: reportNumberOfDTCByStatusMask
io.write("  DTC count  (sub: 0x01, mask: 0xFF) ... ")
io.flush()

local cnt_raw, _, cnt_err = read_dtc(0x01, 0xFF)
local total_dtcs = 0

if cnt_raw then
    -- Response: [PCI][0x59][0x01][availMask][formatID][countH][countL]
    local fmt        = byte_at(cnt_raw, 4)
    total_dtcs       = (byte_at(cnt_raw, 5) << 8) | byte_at(cnt_raw, 6)
    local fmt_str    = DTC_FORMAT_DESC[fmt] or string.format("0x%02X", fmt)
    print(string.format("%d DTC(s)  [Format: %s]", total_dtcs, fmt_str))
else
    print(string.format("FAILED (%s)", cnt_err or "?"))
end

if total_dtcs > 0 then
    -- Sub-function 0x02: reportDTCByStatusMask
    io.write("  DTC list   (sub: 0x02, mask: 0xFF) ... ")
    io.flush()

    local list_raw, list_sf_len, list_err = read_dtc(0x02, 0xFF)
    if list_raw then
        -- Response: [PCI][0x59][0x02][availMask] + n * [DTC_H][DTC_M][DTC_L][status]
        -- Header occupies 3 bytes (SID + sub-fn + availMask); 4 bytes per DTC record.
        local n_in_frame = math.floor((list_sf_len - 3) / 4)
        if n_in_frame == 0 then
            print("(no DTC records fit in single frame)")
        else
            print(string.format("%d DTC(s) in frame:", n_in_frame))
            print(string.format("  %-14s  %-6s  %s", "Code", "Status", "Status Flags"))
            print("  " .. string.rep("─", 74))

            local offset = 4    -- byte offset of first DTC record (0-based)
            for _ = 1, n_in_frame do
                local b1     = byte_at(list_raw, offset)
                local b2     = byte_at(list_raw, offset + 1)
                local b3     = byte_at(list_raw, offset + 2)
                local status = byte_at(list_raw, offset + 3)
                local code   = format_dtc(b1, b2, b3)
                local flags  = {}
                for bit = 0, 7 do
                    if (status >> bit) & 1 == 1 then
                        table.insert(flags, DTC_STATUS[bit])
                    end
                end
                print(string.format("  %-14s  0x%02X    %s",
                    code, status, table.concat(flags, ", ")))
                offset = offset + 4
            end

            if total_dtcs > n_in_frame then
                print(string.format("  (+ %d DTC(s) require multi-frame -- not supported)",
                    total_dtcs - n_in_frame))
            end
        end
    else
        print(string.format("FAILED (%s)", list_err or "?"))
    end
else
    print("  No DTCs stored.")
end

-- ============================================================
--  Done
-- ============================================================

tester_present()
print("")
print(string.rep("─", 76))
print("Readout complete.")
