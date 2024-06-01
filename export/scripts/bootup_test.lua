--[[

Title:   Boot-up test
Author:  Michael Fitzmayer
License: Public domain

This is an example of how a boot-up test could be implemented.

In this test, one device is to be supplied with power via another,
then an SDO is sent to the device under test to check whether it has
booted up properly.  The power-off time varies between 500ms and 60s.
The result is output to a CSV file.

Note: This is a very specific test that must be modified to
accommodate the hardware to be used.  This script only serves as an
example implementation.

--]]

function log(string)
    local f    = io.open("bootup_test_log.csv", "a")
    local date = os.date()
    io.output(f)
    io.write(date..","..string.."\n")
    print(date..","..string)
    io.close(f)
end

print("Boot-up test")

print("Enter node ID (power supply):")
io.write(": ")
node_id_ps = io.read("*n")

node_id_ps_default = 0x40

if (not(node_id_ps)) then
    node_id_ps = node_id_ps_default
end

if (node_id_ps < 1 or node_id_ps > 127) then
    node_id_ps = node_id_ps_default
end

print("Enter node ID (DUT):")
io.write(": ")
node_id_ps = io.read("*n")

node_id_dut_default = 0x78

if (not(node_id_dut)) then
    node_id_dut = node_id_dut_default
end

if (node_id_dut < 1 or node_id_dut > 127) then
    node_id_dut = node_id_ps_default
end

print("How many cycles?")
io.write(": ")
num_cycles = io.read("*n")

num_cycles_default = 100

if (not(num_cycles)) then
    num_cycles = num_cycles_default
end

if (node_id_dut < 1) then
    num_cycles = 1
end

nmt_send_command(node_id_ps, 0x81)
delay_ms(2500)
sdo_write(node_id_ps, 0x210e, 1, 4, 7)
nmt_send_command(node_id_ps, 0x01)

math.randomseed(os.time())

off_time = 0

for i = 1, num_cycles, 1
do
    off_time = math.random(500, 60000)
    sdo_write(node_id_ps, 0x210e, 30, 1, 0)
    delay_ms(off_time)

    sdo_write(node_id_ps, 0x210e, 30, 1, 1)
    delay_ms(45000)

    sdo_read(node_id_dut, 0x1018, 1)

    if sdo_result ~= 0xb3 then
        log(string.format("%d,Failed,%d", i, off_time))
        sdo_reset_result()
        delay_ms(120000)
    else
        log(string.format("%d,OK,%d", i, off_time))
        sdo_reset_result()
    end
end
