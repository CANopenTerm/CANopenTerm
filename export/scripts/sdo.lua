--[[ Simple SDO client example.

Author:  Michael Fitzmayer
License: Public domain

--]]

console_hide()
window_show()
widget_theme(13)
window_resize(640, 480)

local node_id = 0x30
local sdo_index = 0x100a
local sdo_sub_index = 9
local input_valid = true

-- Input widget states.
local input_widgets = {}
local active_input_index = 0
local submitted_input = {}

local function parse_numeric_input(label, text)
    -- Trim leading and trailing whitespace
    text = text:match("^%s*(.-)%s*$")

    if text == "" then
        return nil, "Field cannot be empty."
    end

    -- Only parse these specific fields as numbers.
    if label ~= "Index"
        and label ~= "Sub-Index"
        and label ~= "Node-ID" then
        return text
    end

    -- Determine the base.
    local base = 10
    local value_text = text

    if text:match("^0[xX]") then
        base = 16
        value_text = text:sub(3)

        if value_text == "" then
            return nil, "Hexadecimal value cannot be empty."
        end
    end

    -- Convert to number.
    local value = tonumber(value_text, base)

    if value == nil then
        return nil, "Invalid numeric value: " .. text
    end

    -- Validate range and sign.
    if value < 0 then
        return nil, label .. " cannot be negative."
    end

    -- Require an integer.
    if value % 1 ~= 0 then
        return nil, label .. " must be an integer."
    end

    return value
end

-- Callback function for input widgets.
local function on_input_confirm(input_id_cb, text)
    -- Find which input widget this is.
    for i, widget in ipairs(input_widgets) do
        if widget.id == input_id_cb then
            widget.text = text
            widget.is_active = false
            local read_sdo = false
            local value, error_msg = parse_numeric_input(widget.label, text)

            if value then
                if widget.label == "Node-ID" then
                    if (value < 1 or value > 127) then
                        table.insert(submitted_input, {label = "Error", text = "Node-ID must be in the range 0x01 and 0x7F.", time = os.time()})
                    else                   
                        node_id = value
                        read_sdo = true
                    end
                elseif widget.label == "Index" then
                    if (value < 0 or value > 0xFFFF) then
                        table.insert(submitted_input, {label = "Error", text = "Index must be in the range 0x0000 and 0xFFFF.", time = os.time()})
                    else
                        sdo_index = value
                        read_sdo = true
                    end
                elseif widget.label == "Sub-Index" then
                    if (value < 0 or value > 0xff) then
                        table.insert(submitted_input, {label = "Error", text = "Sub-Index must be in the range 0x00 and 0xFF.", time = os.time()})
                    else
                        sdo_sub_index = value
                        read_sdo = true
                    end
                end

            else
                table.insert(submitted_input, {label = "Error", text = error_msg, time = os.time()})
            end

            if read_sdo then
                result_number, result_text = sdo_read(node_id, sdo_index, sdo_sub_index)

                if result_number == nil then
                    table.insert(submitted_input, {label = "Error", text = "SDO read failed.", time = os.time()})
                else
                    table.insert(submitted_input, {label = "Node-ID", text = string.format("0x%02X", node_id), time = os.time()})
                    table.insert(submitted_input, {label = "Index", text = string.format("0x%04X", sdo_index), time = os.time()})
                    table.insert(submitted_input, {label = "Sub-Index", text = string.format("0x%02X", sdo_sub_index), time = os.time()})
                    table.insert(submitted_input, {label = "Value (numeric)", text = tostring(result_number), time = os.time()})
 
                    if result_text then
                        table.insert(submitted_input, {label = "Value", text = result_text, time = os.time()})
                    end
                end
            end
            break
        end
    end
end

-- Initialize three input widgets.
function init_input_widgets()
    -- Input widget 1: Node-ID. 
    input_widgets[1] = {
        id = widget_input_register(32, 32, 160, 64, tostring(node_id)),
        label = "Node-ID",
        x = 32,
        y = 32,
        width = 160,
        height = 64,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[1].id, on_input_confirm)

    -- Input widget 2: SDO index.
    input_widgets[2] = {
        id = widget_input_register(196, 32, 256, 64, tostring(sdo_index)),
        label = "Index",
        x = 196,
        y = 32,
        width = 256,
        height = 64,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[2].id, on_input_confirm)

    -- Input widget 3: SDO sub-index.
    input_widgets[3] = {
        id = widget_input_register(456, 32, 152, 64, tostring(sdo_sub_index)),
        label = "Sub-Index",
        x = 456,
        y = 32,
        width = 152,
        height = 64,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[3].id, on_input_confirm)
end

-- Handle mouse clicks on input widgets.
function update_input_focus(mouse_x, mouse_y)
    for i, widget in ipairs(input_widgets) do
        if mouse_x >= widget.x and mouse_x <= widget.x + widget.width and
           mouse_y >= widget.y and mouse_y <= widget.y + widget.height then

            -- Deactivate other inputs.
            for j, w in ipairs(input_widgets) do
                if i ~= j then
                    w.is_active = false
                end
            end

            widget.is_active = true
            active_input_index = i
            return
        end
    end

    -- Clicked outside all inputs.
    for i, widget in ipairs(input_widgets) do
        widget.is_active = false
    end
    active_input_index = 0
end

-- Render the GUI.
function render_gui()
    window_clear()

    for i, widget in ipairs(input_widgets) do
        widget.text = widget_input_get_text(widget.id) or ""
        widget.is_active = widget_input_is_active(widget.id)
    end

    -- Input widget 1: Node-ID.
    widget_input(
        math.floor(input_widgets[1].x),
        math.floor(input_widgets[1].y),
        math.floor(input_widgets[1].width),
        math.floor(input_widgets[1].height),
        input_widgets[1].text,
        input_widgets[1].is_active
    )
    widget_print(math.floor(input_widgets[1].x), math.floor(input_widgets[1].y - 20), input_widgets[1].label, 1)

    -- Input widget 2: Index.
    widget_input(
        math.floor(input_widgets[2].x),
        math.floor(input_widgets[2].y),
        math.floor(input_widgets[2].width),
        math.floor(input_widgets[2].height),
        input_widgets[2].text,
        input_widgets[2].is_active
    )
    widget_print(math.floor(input_widgets[2].x), math.floor(input_widgets[2].y - 20), input_widgets[2].label, 1)

    -- Input widget 3: Sub-Index.
    widget_input(
        math.floor(input_widgets[3].x),
        math.floor(input_widgets[3].y),
        math.floor(input_widgets[3].width),
        math.floor(input_widgets[3].height),
        input_widgets[3].text,
        input_widgets[3].is_active
    )
    widget_print(math.floor(input_widgets[3].x), math.floor(input_widgets[3].y - 20), input_widgets[3].label, 1)

    -- Display submitted values.
    widget_print(32, 128, "Output:", 1)

    local y_offset = 141
    if #submitted_input > 0 then
        local display_count = math.min(#submitted_input, 20)
        for i = 1, display_count do
            local idx = #submitted_input - display_count + i
            local entry = submitted_input[idx]
            widget_print(32, y_offset, entry.label .. ": " .. entry.text, 1)
            y_offset = y_offset + 15
        end
    else
        widget_print(32, y_offset, "(No values submitted yet)", 1)
    end
end

init_input_widgets()

while window_is_shown() do
    width, height = window_get_resolution()
    render_gui()
    window_update(true)
end

-- Clean up input widgets.
for i, widget in ipairs(input_widgets) do
    if widget.id and widget.id ~= -1 then
        widget_input_unregister(widget.id)
    end
end

window_clear()
window_hide()
