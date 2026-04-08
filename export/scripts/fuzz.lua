--[[ CAN Bus Fuzzing and Testing Tool

Author:  Michael Fitzmayer
License: Public domain
Comment: Send systematic variations of CAN messages to test ECU robustness.
         This script currently only works with 11-bit CAN IDs.

--]]

local core = require "core"

-- Fuzzing strategies
local FUZZ_STRATEGIES = {
    RANDOM        = 1,
    BIT_FLIP      = 2,
    BOUNDARY      = 3,
    SEQUENTIAL    = 4,
    DLC_VARIATION = 5,
    ALL_ZEROS     = 6,
    ALL_ONES      = 7,
}

local STRATEGY_NAMES = {
    [FUZZ_STRATEGIES.RANDOM]        = "Random data",
    [FUZZ_STRATEGIES.BIT_FLIP]      = "Bit flipping (walk through bits)",
    [FUZZ_STRATEGIES.BOUNDARY]      = "Boundary values (0x00, 0xFF, 0x7F, 0x80)",
    [FUZZ_STRATEGIES.SEQUENTIAL]    = "Sequential values (counting)",
    [FUZZ_STRATEGIES.DLC_VARIATION] = "DLC variation (0-8 bytes)",
    [FUZZ_STRATEGIES.ALL_ZEROS]     = "All zeros (stress test)",
    [FUZZ_STRATEGIES.ALL_ONES]      = "All ones (stress test)",
}

-- Configuration
local target_id       = 0x100
local monitor_id      = nil
local delay_between   = 10
local num_messages    = 100
local strategy        = FUZZ_STRATEGIES.RANDOM
local base_dlc        = 8
local monitor_timeout = 100
local log_responses   = false

-- Statistics
local stats = {
    sent       = 0,
    responses  = 0,
    errors     = 0,
    start_time = 0,
}

-- Seed random number generator
math.randomseed(os.time())

-- Pack a byte-table into a uint64 suitable for can_write().
local function pack_frame(t)
    local data = 0
    for i = 1, math.min(#t, 8) do
        data = data | (t[i] << ((8 - i) * 8))
    end
    return data
end

-- Get the n-th byte (0-based) from raw can_read() data.
local function byte_at(raw, n)
    return (raw >> (n * 8)) & 0xFF
end

-- Format data as hex string for display
local function format_data(data, length)
    local bytes = {}
    for i = length - 1, 0, -1 do
        local byte = (data >> (8 * i)) & 0xFF
        table.insert(bytes, string.format("%02X", byte))
    end
    return table.concat(bytes, " ")
end

-- Generate random data
local function generate_random(dlc)
    local bytes = {}
    for i = 1, dlc do
        table.insert(bytes, math.random(0, 255))
    end
    return pack_frame(bytes), dlc
end

-- Bit flipping: walk through each bit position
local function generate_bit_flip(iteration, dlc)
    local total_bits = dlc * 8
    local bit_pos = (iteration - 1) % total_bits
    local byte_idx = math.floor(bit_pos / 8)
    local bit_in_byte = bit_pos % 8

    local bytes = {}
    for i = 1, dlc do
        if i - 1 == byte_idx then
            table.insert(bytes, 1 << bit_in_byte)
        else
            table.insert(bytes, 0x00)
        end
    end
    return pack_frame(bytes), dlc
end

-- Boundary values: 0x00, 0xFF, 0x7F, 0x80
local function generate_boundary(iteration, dlc)
    local patterns = {0x00, 0xFF, 0x7F, 0x80}
    local pattern = patterns[((iteration - 1) % #patterns) + 1]

    local bytes = {}
    for i = 1, dlc do
        table.insert(bytes, pattern)
    end
    return pack_frame(bytes), dlc
end

-- Sequential values: counting pattern
local function generate_sequential(iteration, dlc)
    local bytes = {}
    local start_val = (iteration - 1) % 256
    for i = 1, dlc do
        table.insert(bytes, (start_val + i - 1) % 256)
    end
    return pack_frame(bytes), dlc
end

-- DLC variation: vary message length
local function generate_dlc_variation(iteration, dlc)
    local current_dlc = ((iteration - 1) % 9)  -- 0 to 8
    local bytes = {}
    for i = 1, current_dlc do
        table.insert(bytes, math.random(0, 255))
    end
    return pack_frame(bytes), current_dlc
end

-- All zeros
local function generate_all_zeros(dlc)
    return 0, dlc
end

-- All ones
local function generate_all_ones(dlc)
    local bytes = {}
    for i = 1, dlc do
        table.insert(bytes, 0xFF)
    end
    return pack_frame(bytes), dlc
end

-- Generate fuzz data based on strategy
local function generate_fuzz_data(iteration)
    if strategy == FUZZ_STRATEGIES.RANDOM then
        return generate_random(base_dlc)
    elseif strategy == FUZZ_STRATEGIES.BIT_FLIP then
        return generate_bit_flip(iteration, base_dlc)
    elseif strategy == FUZZ_STRATEGIES.BOUNDARY then
        return generate_boundary(iteration, base_dlc)
    elseif strategy == FUZZ_STRATEGIES.SEQUENTIAL then
        return generate_sequential(iteration, base_dlc)
    elseif strategy == FUZZ_STRATEGIES.DLC_VARIATION then
        return generate_dlc_variation(iteration, base_dlc)
    elseif strategy == FUZZ_STRATEGIES.ALL_ZEROS then
        return generate_all_zeros(base_dlc)
    elseif strategy == FUZZ_STRATEGIES.ALL_ONES then
        return generate_all_ones(base_dlc)
    else
        return generate_random(base_dlc)
    end
end

-- Monitor for responses
local function check_for_response()
    local deadline = os.clock() + monitor_timeout / 1000
    local responses = {}

    while os.clock() < deadline do
        local id, length, data = can_read()
        if id and (not monitor_id or id == monitor_id) then
            table.insert(responses, {
                id = id,
                length = length,
                data = data
            })
        end
    end

    return responses
end

-- Display statistics
local function display_stats()
    local elapsed = os.clock() - stats.start_time
    local rate = stats.sent / elapsed

    io.write(string.format("\r[%5d sent | %4d responses | %3d errors | %.1f msg/s]",
        stats.sent, stats.responses, stats.errors, rate))
    io.flush()
end

-- Log response details
local function log_response(iteration, responses)
    if #responses > 0 then
        print(string.format("\n  Iteration %d triggered %d response(s):", iteration, #responses))
        for _, resp in ipairs(responses) do
            print(string.format("    ID: 0x%03X  DLC: %d  Data: %s  Time: %d μs",
                resp.id, resp.length, format_data(resp.data, resp.length), resp.timestamp or 0))
        end
    end
end

-- Main fuzzing loop
local function run_fuzzer()
    can_flush()
    stats.start_time = os.clock()

    print("\nStarting fuzzing session...")
    print(string.format("Strategy: %s", STRATEGY_NAMES[strategy]))
    print(string.format("Target ID: 0x%03X", target_id))
    if monitor_id then
        print(string.format("Monitoring ID: 0x%03X", monitor_id))
    end
    print("\nPress any key to stop.\n")

    local iteration = 1
    while (num_messages == 0 or iteration <= num_messages) and not key_is_hit() do
        -- Generate fuzz data
        local data, dlc = generate_fuzz_data(iteration)

        -- Send message
        local success = can_write(target_id, dlc, data)
        if success then
            stats.sent = stats.sent + 1
        else
            stats.errors = stats.errors + 1
        end

        -- Check for responses
        if monitor_id or log_responses then
            local responses = check_for_response()
            if #responses > 0 then
                stats.responses = stats.responses + #responses
                if log_responses then
                    log_response(iteration, responses)
                end
            end
        end

        -- Display progress
        if iteration % 10 == 0 then
            display_stats()
        end

        -- Delay between messages
        if delay_between > 0 then
            delay_ms(delay_between)
        end

        iteration = iteration + 1
    end

    -- Final statistics
    display_stats()
    print("\n")

    local elapsed = os.clock() - stats.start_time
    print(string.rep("─", 60))
    print("Fuzzing session complete.")
    print(string.format("Total messages sent: %d", stats.sent))
    print(string.format("Total responses: %d", stats.responses))
    print(string.format("Total errors: %d", stats.errors))
    print(string.format("Duration: %.2f seconds", elapsed))
    print(string.format("Average rate: %.1f msg/s", stats.sent / elapsed))
    print(string.rep("─", 60))
end

-- ============================================================
--  Setup
-- ============================================================

print("═══════════════════════════════════════════════════════════")
print("  CAN Bus Fuzzing and Testing Tool")
print("═══════════════════════════════════════════════════════════")

local inp = core.select_variable("Target CAN-ID in hex (e.g., 100)")
if inp == nil then print("Exiting.") return end
if inp ~= "" then
    local val = tonumber(inp, 16)
    if val then target_id = val end
end

inp = core.select_variable("Monitor response from CAN-ID (hex, leave empty to monitor all)")
if inp == nil then print("Exiting.") return end
if inp ~= "" then
    local val = tonumber(inp, 16)
    if val then monitor_id = val end
end

print("\nFuzzing Strategies:")
print("  1. Random data")
print("  2. Bit flipping (walk through bits)")
print("  3. Boundary values (0x00, 0xFF, 0x7F, 0x80)")
print("  4. Sequential values (counting)")
print("  5. DLC variation (0-8 bytes)")
print("  6. All zeros (stress test)")
print("  7. All ones (stress test)")

local strat = core.select_number("Select fuzzing strategy (1-7):")
if strat == nil then print("Exiting.") return end
if strat >= 1 and strat <= 7 then
    strategy = strat
end

if strategy ~= FUZZ_STRATEGIES.DLC_VARIATION then
    local dlc_inp = core.select_number("Data Length Code (DLC, 0-8, default: 8):")
    if dlc_inp == nil then print("Exiting.") return end
    if dlc_inp >= 1 and dlc_inp <= 8 then
        base_dlc = dlc_inp
    end
    -- If invalid or empty (0), keep default of 8
end

local delay_inp = core.select_number("Delay between messages in ms (default: 10):")
if delay_inp == nil then print("Exiting.") return end
if delay_inp >= 0 then
    delay_between = delay_inp
end

local count_inp = core.select_number("Number of messages to send (0 for continuous):")
if count_inp == nil then print("Exiting.") return end
if count_inp >= 0 then
    num_messages = count_inp
end

inp = core.select_variable("Log detailed response information? [y/N]")
if inp == nil then print("Exiting.") return end
log_responses = (inp:lower() == "y" or inp:lower() == "yes")

-- Run the fuzzer
run_fuzzer()
