--[[ A simple car dashboard example.

Author:  Michael Fitzmayer
License: Public domain

This script implements a real-time automotive dashboard that displays vehicle
speed and engine RPM by monitoring OBD-II diagnostic data over CAN bus.

Implementation notes:
- Supports single-frame OBD-II responses only (no ISO-TP multi-frame handling)
- Implements SAE J1979 Mode 0x01 (Show Current Data) for PIDs 0x0C and 0x0D
- Based on ISO 15031 OBD-II data format and SAE J1979 specifications
- Limited to basic diagnostic data (speed and RPM)

OBD-II test frames (inject via CAN for testing):
- Speed (100 km/h): id=0x7E8, length=8, data=0x02410D6400000000
- RPM (3000 rpm):   id=0x7E8, length=8, data=0x04410C2EE0000000

--]]

-- Hide the console and show the graphical window.
console_hide()
window_show()

-- Global state variables.
offset = 10              -- Horizontal offset for widget positioning.
previous_data = nil      -- Cache previous CAN data to avoid redundant processing.
kmh = 0                  -- Current vehicle speed in km/h.
rpm = 0                  -- Current engine RPM.
first_render = true      -- Flag to force initial rendering.

-- OBD-II standard response CAN ID.
-- All OBD-II responses use this CAN ID for diagnostic data.
OBD_RESPONSE_ID = 0x7E8

--[[ Parse vehicle speed from OBD-II data.

     This function extracts the vehicle speed from an OBD-II response frame
     for PID 0x0D (Vehicle Speed).

     OBD-II data format (standard big-endian):
     - Byte 0:     Length (number of data bytes following)
     - Byte 1:     Mode (0x41 for response to mode 0x01)
     - Byte 2:     PID (0x0D for vehicle speed)
     - Byte 3:     Data byte A (speed value in km/h)

     Example: 0x02410D6400000000
              ^^          Byte 0: Length = 2 bytes
                ^^        Byte 1: Mode   = 0x41 (response)
                  ^^      Byte 2: PID    = 0x0D (vehicle speed)
                    ^^    Byte 3: Data   = 0x64 (100 km/h)

     @param data: OBD-II response frame (64-bit integer)
     @return: Vehicle speed in km/h (0-255)
--]]
function parse_vehicle_speed(data)
    -- Shift right by 24 bits to move byte 3 to the least significant position.
    -- Then mask with 0xff to extract only the speed byte.
    return (data >> 24) & 0xff
end

--[[ Parse engine RPM from OBD-II data.

     This function extracts the engine RPM from an OBD-II response frame
     for PID 0x0C (Engine RPM).

     OBD-II data format (standard big-endian):
     - Byte 0:     Length (number of data bytes following)
     - Byte 1:     Mode (0x41 for response to mode 0x01)
     - Byte 2:     PID (0x0C for engine RPM)
     - Byte 3:     Data byte A (high byte of RPM value)
     - Byte 4:     Data byte B (low byte of RPM value)

     Example: 0x04410C2EE0000000
              ^^            Byte 0: Length = 4 bytes
                ^^          Byte 1: Mode   = 0x41 (response)
                  ^^        Byte 2: PID    = 0x0C (engine RPM)
                    ^^      Byte 3: A      = 0x2E (high byte)
                      ^^    Byte 4: B      = 0xE0 (low byte)

     The RPM is calculated using the formula: ((A * 256) + B) / 4
     This converts the two-byte value to actual RPM.

     @param data: OBD-II response frame (64-bit integer)
     @return: Engine RPM in revolutions per minute (0-16383.75)
--]]
function parse_engine_rpm(data)
    -- Extract byte 3 (high byte A) by shifting right 24 bits.
    local a = (data >> 24) & 0xff
    -- Extract byte 4 (low byte B) by shifting right 32 bits.
    local b = (data >> 32) & 0xff
    -- Combine bytes and apply OBD-II formula: ((A * 256) + B) / 4
    return ((a * 256) + b) / 4
end

local last_request_time = 0
local request_interval = 0.25 -- seconds
local request_rpm = true      -- Toggle between RPM and Speed requests

-- Main application loop - continues while window is visible.
-- This loop repeatedly reads CAN data, updates state, and redraws the dashboard.
while window_is_shown() do
    local now = os.clock()

    -- Periodically send OBD-II requests (alternate between RPM and Speed).
    if now - last_request_time > request_interval then
        if request_rpm then
            can_write(0x7E0, 8, 0x02010C0000000000)  -- RPM.
        else
            can_write(0x7E0, 8, 0x02010D0000000000)  -- Speed.
        end
        request_rpm = not request_rpm  -- Toggle for next cycle.
        last_request_time = now
    end

    -- Read CAN data and get current window dimensions.
    -- can_read() returns: CAN ID, data length, and data payload.
    id, length, data = can_read()
    -- Get window dimensions for responsive layout calculations.
    width, height = window_get_resolution()
    -- Flag to indicate if we need to redraw the screen this iteration.
    render = false

    -- Calculate widget sizes and positions based on window dimensions.
    -- This creates a responsive layout that adapts to window size.
    size = width / 3                         -- Tachometer size (1/3 of window width).
    spacing = width / 6                      -- Spacing from edges (1/6 of window width).
    pos_y = (height / 2) - (size / 2)        -- Vertical center position.
    
    -- Position for KM/H gauge (left side).
    -- Layout: [spacing][gauge][spacing][spacing][gauge][spacing]
    pos_x_kmh = spacing
    -- Position for RPM gauge (right side).
    pos_x_rpm = width - spacing - size

    -- Parse OBD-II data if new data is available and from correct CAN ID.
    -- This section filters for valid OBD-II responses and prevents reprocessing.
    if id ~= nil and id == OBD_RESPONSE_ID and data ~= nil and data ~= previous_data then
        -- Extract mode and PID from OBD-II response frame.
        -- OBD-II standard format: [Length, Mode+0x40, PID, Data...]
        -- The mode byte is at position 1 (shift right 8 bits).
        local mode = (data >> 8) & 0xff
        -- The PID byte is at position 2 (shift right 16 bits).
        local pid = (data >> 16) & 0xff
        
        -- Check if this is a response to mode 0x01 (show current data).
        -- Mode 0x01 requests are responded to with mode 0x41 (0x01 + 0x40).
        if mode == 0x41 then  -- Response to mode 0x01 (show current data).
            if pid == 0x0D then  -- Vehicle speed PID.
                -- Update speed value and trigger redraw.
                kmh = parse_vehicle_speed(data)
                render = true
            elseif pid == 0x0C then  -- Engine RPM PID.
                -- Update RPM value and trigger redraw.
                rpm = parse_engine_rpm(data)
                render = true
            end
        end
        
        -- Update data cache to prevent reprocessing.
        -- This optimization avoids redundant parsing of the same CAN frame.
        previous_data = data
    end

    -- Force initial render to display gauges on startup.
    -- This ensures the dashboard is visible even before receiving CAN data.
    if first_render then
        render = true
        first_render = false
    end

    -- Redraw the dashboard if new data was received or on first run.
    -- This conditional rendering improves performance by only updating when needed.
    if render then
        -- Clear the window to prepare for fresh drawing.
        window_clear()
        
        -- Display application title at the top-left corner.
        widget_print(10, 10, "Diagnostics: All Systems Nerdy", 3)

        -- Display current speed and RPM values as text (digital readout).
        -- Format numbers with fixed width for consistent alignment.
        widget_print(10, 50, string.format("%3d", kmh) .. "  km/h", 4)
        widget_print(10, 100, string.format("%4d", rpm) .. " RPM", 4)

        -- Display gauge labels above each tachometer.
        widget_print(pos_x_kmh - offset, pos_y - 10, "KM/H", 2)
        widget_print(pos_x_rpm + offset, pos_y - 10, "RPM", 2)

        -- Display tachometer gauges (analog visualization).
        -- Parameters: x, y, size, max_value, current_value
        -- Max speed: 260 km/h, Max RPM: 8000 (typical car redline).
        widget_tachometer(pos_x_kmh - offset, pos_y, size, 260, kmh)
        widget_tachometer(pos_x_rpm + offset, pos_y, size, 8000, rpm)
    end

    -- Update window display (only if render flag is set).
    -- Passing the render flag tells the system whether to actually refresh the screen.
    -- This prevents unnecessary screen updates when data hasn't changed.
    window_update(render)
end

-- Clean up: clear and hide window on exit.
-- This ensures a clean shutdown when the user closes the window.
window_clear()
window_hide()
