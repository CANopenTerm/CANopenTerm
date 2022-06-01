-- Reset node
nmt_send_command(0x50, 0x81)
delay_ms(200)

-- Read and output 0x2100, 1
sdo_read(0x50, 0x2100, 1)
print(sdo_result)

-- Update, read and output 0x2100, 1
sdo_write(0x50, 0x2100, 1, 4, 1)
sdo_read(0x50, 0x2100, 1)
print(sdo_result)
