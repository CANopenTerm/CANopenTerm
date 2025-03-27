--[[ Widget demo

Author:  Michael Fitzmayer
License: Public domain

--]]

show_window()

previous_data = 0

while not key_is_hit() do
    id, length, data = can_read()

    if data == nil then
        data = previous_data
    else
        previous_data = data
    end

    if (id == 0x100) then
        clear_window()
        widget_tachometer(10, 10, 100, 5000, data)
        update_window()
    end
end

clear_window()
hide_window()
