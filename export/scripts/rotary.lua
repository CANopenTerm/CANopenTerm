--[[ Rotary encoder visualisation

Author:  Michael Fitzmayer
License: Public domain

--]]

console_hide()
window_show()

previous_data = -1
previous_value3 = 0
value3 = 0

while window_is_shown() do
    id, length, data = can_read()
    width, height = window_get_resolution()
    render = false

    size = width / 4
    led = size / 10
    pos_x = (width / 2) - (size / 2)
    pos_y = (height / 2) - (size / 2)

    if data == nil then
        data = previous_data
    else
        if id == 0x191 and previous_data ~= data then
            window_clear()

            widget_print(10, 10, "Rotary encoder", 3)
            previous_data = data
			local value1 = data & 0xffff
			local value2 = (data >> 16) & 0xffff

			previous_value3 = value3
			value3 = (data >> 32) & 0xffff

            local value4 = (data >> 48) & 0xff
            local button1 = (value4 & 0x01) ~= 0
            local button2 = (value4 & 0x02) ~= 0
            local button3 = (value4 & 0x04) ~= 0

			if previous_value3 < value3 then
				for i = 1, 3 do key_send(0x28) end
			elseif previous_value3 > value3 then
				for i = 1, 3 do key_send(0x26) end
			end

            widget_led(pos_x - size - 10, pos_y, led, button1)
            widget_led(pos_x, pos_y, led, button2)
            widget_led(pos_x + size + 10, pos_y, led, button3)

            widget_tachometer(pos_x - size - 10, pos_y, size, 0x40, value1)
            widget_tachometer(pos_x, pos_y, size, 0x40, value2)
            widget_tachometer(pos_x + size + 10, pos_y, size, 0x40, value3)

            render = true
        end
    end

    window_update(render)
end

window_clear()
window_hide()
