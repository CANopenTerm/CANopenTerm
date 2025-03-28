--[[ Widget demo

Author:  Michael Fitzmayer
License: Public domain

--]]

show_window()

previous_data = 0
value = 0
dir = 0

while not key_is_hit() do
    id, length, data = can_read()

    if data == nil then
        data = previous_data
    else
        if id == 0x100 then
           previous_data = data
        end
    end

    if value >= 5000 then
        dir = 1
    elseif value <= 0 then
        dir = 0
    end

    if dir == 0 then
        value = value + 10
    else
        value = value - 10
    end

    clear_window()
    widget_tachometer(10, 10, 100, 5000, data)
    widget_status_bar(230, 10, 100, 25, 5000, data)
    widget_tachometer(120, 10, 100, 5000, value)
    widget_status_bar(230, 45, 100, 25, 5000, value)
    update_window()
end

clear_window()
hide_window()
