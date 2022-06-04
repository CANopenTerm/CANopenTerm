-- Chaos generator

function print_progress(progress)
    io.write("\rProgress: ")
    io.write(progress)
    io.write("%")
end

counter  = 0
progress = 0

math.randomseed(os.time())

print("How many messages should be sent?")
io.write(": ")
msg_count = io.read("*n")

if (not(msg_count))
then
    msg_count = 100
end

print("Please enter the interval in milliseconds.")
io.write(": ")
msg_interval = io.read("*n")

if (not(msg_interval))
then
    msg_interval = 0
end

print("Generating chaos.")
print_progress(progress)

for i = 1, msg_count, 1
do
    node_id    = math.random(0x7f)
    data_d0_d3 = math.random(0xffffffff)
    data_d4_d7 = math.random(0xffffffff)

    can_write(node_id, 8, data_d0_d3, data_d0_d3);

    counter = counter + 1
    if(counter >= (msg_count / 100))
    then
        progress = progress + 1
        counter  = 0
        print_progress(progress)
    end

    delay_ms(msg_interval)
end
io.write("\n")
io.flush()
