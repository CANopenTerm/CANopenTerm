--[[ Input widget demo

Author:  Michael Fitzmayer
License: Public domain

--]]

console_hide()
window_show()
widget_theme(5)
window_resize(800, 450)

delay_ms(100)

-- Input widget states
local input_widgets = {}
local active_input_index = 0

-- Demo data to display
local submitted_texts = {}

-- Callback function for input widgets (declare before use)
local function on_input_confirm(input_id_cb, text)
    -- Find which input widget this is
    for i, widget in ipairs(input_widgets) do
        if widget.id == input_id_cb then
            widget.text = text
            widget.is_active = false
            table.insert(submitted_texts, {label = widget.label, text = text, time = os.time()})
            print("Input " .. i .. " (" .. widget.label .. "): " .. text)
            break
        end
    end
end

-- Initialize three input widgets
function init_input_widgets()
    -- Input widget 1: Username (top left)
    input_widgets[1] = {
        id = widget_input_register(50, 50, 260, 40, "admin"),
        label = "Username",
        x = 50,
        y = 50,
        width = 260,
        height = 40,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[1].id, on_input_confirm)

    -- Input widget 2: Email (top right)
    input_widgets[2] = {
        id = widget_input_register(340, 50, 370, 40, "root@canopenterm.de"),
        label = "Email",
        x = 340,
        y = 50,
        width = 370,
        height = 40,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[2].id, on_input_confirm)

    -- Input widget 3: Message (full width, lower)
    input_widgets[3] = {
        id = widget_input_register(50, 130, 700, 100, "Hello world."),
        label = "Message",
        x = 50,
        y = 130,
        width = 700,
        height = 100,
        text = "",
        is_active = false
    }
    widget_input_set_callback(input_widgets[3].id, on_input_confirm)
end

-- Handle mouse clicks on input widgets
function update_input_focus(mouse_x, mouse_y)
    for i, widget in ipairs(input_widgets) do
        if mouse_x >= widget.x and mouse_x <= widget.x + widget.width and
           mouse_y >= widget.y and mouse_y <= widget.y + widget.height then

            -- Deactivate other inputs
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

    -- Clicked outside all inputs
    for i, widget in ipairs(input_widgets) do
        widget.is_active = false
    end
    active_input_index = 0
end

-- Render the demo interface
function render_input_demo()
    window_clear()

    -- Update text and active state from C widget for each widget
    for i, widget in ipairs(input_widgets) do
        widget.text = widget_input_get_text(widget.id) or ""
        widget.is_active = widget_input_is_active(widget.id)
    end

    -- Input widget 1: Username
    widget_input(
        math.floor(input_widgets[1].x),
        math.floor(input_widgets[1].y),
        math.floor(input_widgets[1].width),
        math.floor(input_widgets[1].height),
        input_widgets[1].text,
        input_widgets[1].is_active
    )
    widget_print(math.floor(input_widgets[1].x), math.floor(input_widgets[1].y - 20), input_widgets[1].label, 1)

    -- Input widget 2: Email
    widget_input(
        math.floor(input_widgets[2].x),
        math.floor(input_widgets[2].y),
        math.floor(input_widgets[2].width),
        math.floor(input_widgets[2].height),
        input_widgets[2].text,
        input_widgets[2].is_active
    )
    widget_print(math.floor(input_widgets[2].x), math.floor(input_widgets[2].y - 20), input_widgets[2].label, 1)

    -- Input widget 3: Message
    widget_input(
        math.floor(input_widgets[3].x),
        math.floor(input_widgets[3].y),
        math.floor(input_widgets[3].width),
        math.floor(input_widgets[3].height),
        input_widgets[3].text,
        input_widgets[3].is_active
    )
    widget_print(math.floor(input_widgets[3].x), math.floor(input_widgets[3].y - 20), input_widgets[3].label, 1)

    -- Instructions
    widget_print(50, 250, "INSTRUCTIONS:", 1)
    widget_print(50, 270, "Click on any input field to enter text", 1)
    widget_print(50, 285, "Type text using your keyboard", 1)
    widget_print(50, 300, "Press BACKSPACE to delete characters", 1)
    widget_print(50, 315, "Press ENTER to confirm input", 1)

    -- Display submitted values
    widget_print(50, 340, "SUBMITTED VALUES:", 1)

    local y_offset = 360
    if #submitted_texts > 0 then
        local display_count = math.min(#submitted_texts, 5)
        for i = 1, display_count do
            local idx = #submitted_texts - display_count + i
            local entry = submitted_texts[idx]
            widget_print(70, y_offset, entry.label .. ": " .. entry.text, 1)
            y_offset = y_offset + 15
        end
    else
        widget_print(70, y_offset, "(No values submitted yet)", 1)
    end
end

-- Main loop
init_input_widgets()

while window_is_shown() do
    width, height = window_get_resolution()

    render_input_demo()

    window_update(true)
end

-- Clean up input widgets
for i, widget in ipairs(input_widgets) do
    if widget.id and widget.id ~= -1 then
        widget_input_unregister(widget.id)
    end
end

window_clear()
window_hide()
