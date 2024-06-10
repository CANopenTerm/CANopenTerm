--[[

Title:   Progress display
Author:  Michael Fitzmayer
License: Public domain

--]]

progress = 0
for i = 1, 100, 1
do
    progress = progress + 1

    io.write("\rProgress: ")
    io.write(progress)
    io.write("%")

    if (key_is_hit())
    then
        break;
    end

    delay_ms(100)
end
io.write('\n')
io.flush()
