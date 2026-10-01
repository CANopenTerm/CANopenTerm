--[[ Widget demo

Author:  Michael Fitzmayer
License: Public domain

--]]

console_hide()
window_show()
widget_theme(1)
window_resize(900, 900)

delay_ms(100)

demo_time = 0
led_states = {true, true, false, true}
led_index = 1
led_toggle_time = 0

local osc_id1 = oscilloscope_buffer_create(0, 100, 2048)
local osc_id2 = oscilloscope_buffer_create(0, 100, 2048)

function calculate_text_center(text_length, total_width, char_scale)
    local char_width = 5  -- Base character width
    local char_spacing = 1
    local text_width = (char_width + char_spacing) * char_scale * text_length - char_spacing
    return total_width / 2 - text_width / 2
end

function update_led_state()
    led_toggle_time = led_toggle_time + 1
    if led_toggle_time >= 60 then
        led_index = (led_index % #led_states) + 1
        led_toggle_time = 0
    end
    return led_states[led_index]
end

function animate_value(max, speed)
    local phase = (demo_time * speed) % (2 * 3.14159)
    local normalized = (math.sin(phase) + 1) / 2  -- Range 0 to 1
    return math.floor(normalized * max)
end

function animate_value_offset(max, speed, offset)
    local phase = (demo_time * speed + offset) % (2 * 3.14159)
    local normalized = (math.sin(phase) + 1) / 2  -- Range 0 to 1
    return math.floor(normalized * max)
end

function calculate_layout(width, height)
    if width == 0 or height == 0 then
        width = 1200
        height = 1000
    end

    local layout = {}

    local margin_top = 20
    local margin_sides = 30
    local section_spacing = 60

    layout.title_y = margin_top
    layout.title_x = margin_sides

    layout.led_section_y = layout.title_y + 60
    layout.led_size = 60
    layout.led_spacing_x = (width - (margin_sides * 2)) / 3
    layout.led_1_x = margin_sides + layout.led_spacing_x / 2 - layout.led_size / 2
    layout.led_2_x = margin_sides + (layout.led_spacing_x * 1.5) - layout.led_size / 2
    layout.led_3_x = margin_sides + (layout.led_spacing_x * 2.5) - layout.led_size / 2
    layout.led_label_y = layout.led_section_y + layout.led_size + 8

    layout.tach_section_y = layout.led_label_y + section_spacing
    layout.tachometer_size = 150
    layout.tach_1_x = margin_sides + (width - margin_sides * 2) / 4 - layout.tachometer_size / 2
    layout.tach_2_x = margin_sides + (width - margin_sides * 2) * 3 / 4 - layout.tachometer_size / 2

    layout.bargraph_y = layout.tach_section_y + layout.tachometer_size + section_spacing
    layout.bargraph_width = (width - margin_sides * 2) / 2 - 10
    layout.bargraph_height = 40
    layout.bargraph_1_x = margin_sides
    layout.bargraph_2_x = margin_sides + layout.bargraph_width + 20

    layout.osc_y = layout.bargraph_y + layout.bargraph_height + section_spacing
    layout.osc_width = width - (margin_sides * 2)
    layout.osc_height = 150
    layout.osc_x = margin_sides

    layout.osc2ch_y = layout.osc_y + layout.osc_height + section_spacing
    layout.osc2ch_width = width - (margin_sides * 2)
    layout.osc2ch_height = 150
    layout.osc2ch_x = margin_sides

    return layout
end

function render_demo(layout, led_state, tach_val_1, tach_val_2, bar_val_1, bar_val_2)
    window_clear()
    widget_print(math.floor(layout.title_x), math.floor(layout.title_y), "WIDGET DEMO", 3)
    widget_print(math.floor(layout.title_x), math.floor(layout.led_section_y - 25), "LED INDICATORS", 2)

    widget_led(math.floor(layout.led_1_x), math.floor(layout.led_section_y), layout.led_size, led_state)
    widget_print(math.floor(layout.led_1_x - 5), math.floor(layout.led_label_y), "Status", 1)

    widget_led(math.floor(layout.led_2_x), math.floor(layout.led_section_y), layout.led_size, true)
    widget_print(math.floor(layout.led_2_x + 5), math.floor(layout.led_label_y), "Power", 1)

    widget_led(math.floor(layout.led_3_x), math.floor(layout.led_section_y), layout.led_size, false)
    widget_print(math.floor(layout.led_3_x + 5), math.floor(layout.led_label_y), "Error", 1)

    widget_print(math.floor(layout.title_x), math.floor(layout.tach_section_y - 35), "TACHOMETERS", 2)

    local tach1_center_x = layout.tach_1_x + layout.tachometer_size / 2
    widget_print(math.floor(tach1_center_x - 15), math.floor(layout.tach_section_y - 50), "SPEED", 1)
    widget_print(math.floor(tach1_center_x - 30), math.floor(layout.tach_section_y - 35), "0-260 km/h", 1)
    widget_tachometer(
        math.floor(layout.tach_1_x),
        math.floor(layout.tach_section_y),
        layout.tachometer_size,
        260,
        tach_val_1
    )

    local tach2_center_x = layout.tach_2_x + layout.tachometer_size / 2
    widget_print(math.floor(tach2_center_x - 10), math.floor(layout.tach_section_y - 50), "RPM", 1)
    widget_print(math.floor(tach2_center_x - 30), math.floor(layout.tach_section_y - 35), "0-8000 rpm", 1)
    widget_tachometer(
        math.floor(layout.tach_2_x),
        math.floor(layout.tach_section_y),
        layout.tachometer_size,
        8000,
        tach_val_2
    )

    widget_print(math.floor(layout.title_x), math.floor(layout.bargraph_y - 35), "BARGRAPHS", 2)

    widget_print(math.floor(layout.bargraph_1_x), math.floor(layout.bargraph_y - 15), "Energy", 1)
    widget_bargraph(
        math.floor(layout.bargraph_1_x),
        math.floor(layout.bargraph_y),
        layout.bargraph_width,
        layout.bargraph_height,
        100,
        bar_val_1
    )

    widget_print(math.floor(layout.bargraph_2_x), math.floor(layout.bargraph_y - 15), "Signal", 1)
    widget_bargraph(
        math.floor(layout.bargraph_2_x),
        math.floor(layout.bargraph_y),
        layout.bargraph_width,
        layout.bargraph_height,
        100,
        bar_val_2
    )
end

function render_demo_with_osc(layout, led_state, tach_val_1, tach_val_2, bar_val_1, bar_val_2, 
                               osc_id1, osc_val1, osc_id2, osc_val2)
    render_demo(layout, led_state, tach_val_1, tach_val_2, bar_val_1, bar_val_2)

    widget_print(math.floor(layout.title_x), math.floor(layout.osc_y - 25), "OSCILLOSCOPE", 2)

    widget_oscilloscope(
        math.floor(layout.osc_x),
        math.floor(layout.osc_y),
        layout.osc_width,
        layout.osc_height,
        osc_id1,
        osc_val1,
        "Waveform Display (0-100)",
        1000
    )

    widget_print(math.floor(layout.title_x), math.floor(layout.osc2ch_y - 25), "DUAL-CHANNEL OSCILLOSCOPE", 2)

    widget_oscilloscope_2ch(
        math.floor(layout.osc2ch_x),
        math.floor(layout.osc2ch_y),
        layout.osc2ch_width,
        layout.osc2ch_height,
        osc_id1,
        osc_val1,
        "Ch1: Sine Wave (10ms)",
        osc_id2,
        osc_val2,
        "Ch2: Triangle Wave (50ms)",
        500
    )
end

while window_is_shown() do
    demo_time = demo_time + 0.016 -- ~60 FPS update

    width, height = window_get_resolution()
    layout = calculate_layout(width, height)

    led_state = update_led_state()

    tach_val_1 = animate_value(260, 0.02)               -- Speed: 0-260
    tach_val_2 = animate_value_offset(8000, 0.03, 1.57) -- RPM: 0-8000, phase offset
    bar_val_1 = animate_value(100, 0.025)               -- Energy: 0-100
    bar_val_2 = animate_value_offset(100, 0.02, 3.14)   -- Signal: 0-100, phase offset

    local sine_component = 30 * math.sin(demo_time / 10)
    local noise = math.random(-8, 8)
    local osc_val_1 = 50 + sine_component + noise
    osc_val_1 = math.max(0, math.min(100, math.floor(osc_val_1)))
    oscilloscope_buffer_push(osc_id1, osc_val_1)

    local triangle_component = 25 * (math.abs((demo_time % 50) - 25) / 25 - 1)
    local noise2 = math.random(-5, 5)
    local osc_val_2 = 50 + triangle_component + noise2
    osc_val_2 = math.max(0, math.min(100, math.floor(osc_val_2)))
    oscilloscope_buffer_push(osc_id2, osc_val_2)

    render_demo_with_osc(layout, led_state, tach_val_1, tach_val_2, bar_val_1, bar_val_2, 
                         osc_id1, osc_val_1, osc_id2, osc_val_2)

    window_update(true)
end

oscilloscope_buffer_destroy(osc_id1)
oscilloscope_buffer_destroy(osc_id2)
window_clear()
window_hide()
