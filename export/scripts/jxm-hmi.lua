--[[ Jetter JXM-HMI

Author:  Michael Fitzmayer
License: Public domain

--]]

window_show()

previous_data = -1

while not key_is_hit() do
    id, length, data = can_read()
    width, height = window_get_resolution()
    render = false

    size = width / 4;
    pos_x = (width / 2) - (size / 2)
    pos_y = (height / 2) - (size / 2)

    if data == nil then
        data = previous_data
    else
        if id == 0x191 and previous_data ~= data then
            window_clear()
            previous_data = data
			value1 = data & 0xffff
			value2 = (data >> 16) & 0xffff
			value3 = (data >> 32) & 0xffff

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
