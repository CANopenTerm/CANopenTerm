--[[ An OBD-II speed and RPM monitor

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

If the service mode or PID is unsupported, the response will indicate an
error (e.g., 0x7F for negative response).  Especially older vehicles may not
support all PIDs or use K-line instead of CAN, so compatibility may vary.
Always verify supported PIDs.

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

-- OBD-II standard request CAN ID.
-- All OBD-II requests use this CAN ID to query diagnostic data.
OBD_REQUEST_ID = 0x7E0

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
local request_interval = 0.25 -- Request OBD data every 250 milliseconds.
local request_rpm = true

function send_obd_request(now)
    if now - last_request_time > request_interval then
        if request_rpm then
            can_write(OBD_REQUEST_ID, 8, 0x02010C0000000000)
        else
            can_write(OBD_REQUEST_ID, 8, 0x02010D0000000000)
        end
        request_rpm = not request_rpm
        last_request_time = now
    end
end

function process_obd_response(id, data)
    if id ~= nil and id == OBD_RESPONSE_ID and
       data ~= nil and data ~= previous_data then

        local mode = (data >> 8) & 0xff
        local pid = (data >> 16) & 0xff

        if mode == 0x41 then
            if pid == 0x0D then
                kmh = parse_vehicle_speed(data)
                return true
            elseif pid == 0x0C then
                rpm = parse_engine_rpm(data)
                return true
            end
        end

        previous_data = data
    end
    return false
end

function calculate_layout(width, height)
    local layout = {}
    layout.size = width / 3
    layout.spacing = width / 6
    layout.pos_y = (height / 2) - (layout.size / 2)
    layout.pos_x_kmh = layout.spacing
    layout.pos_x_rpm = width - layout.spacing - layout.size
    return layout
end

function render_dashboard(layout)
    window_clear()

    widget_print(10, 10, "CAR DASHBOARD", 4)
    widget_print(10, 75, string.format("%4d", kmh) .. " km/h", 4)
    widget_print(10, 125, string.format("%4d", rpm) .. " RPM", 4)

    widget_print(layout.pos_x_kmh - offset, layout.pos_y - 10, "KM/H", 2)
    widget_print(layout.pos_x_rpm + offset, layout.pos_y - 10, "RPM", 2)

    widget_tachometer(layout.pos_x_kmh - offset, layout.pos_y, layout.size, 260, kmh)
    widget_tachometer(layout.pos_x_rpm + offset, layout.pos_y, layout.size, 8000, rpm)
end

-- Main application loop - continues while window is visible.
-- This loop repeatedly reads CAN data, updates state, and redraws the dashboard.
while window_is_shown() do
    local now = os.clock()
    send_obd_request(now)

    id, length, data = can_read()
    width, height = window_get_resolution()
    layout = calculate_layout(width, height)

    render = process_obd_response(id, data)

    if first_render then
        render = true
        first_render = false
    end

    if render then
        render_dashboard(layout)
    end

    window_update(render)
end

-- Clean up: clear and hide window on exit.
-- This ensures a clean shutdown when the user closes the window.
window_clear()
window_hide()
