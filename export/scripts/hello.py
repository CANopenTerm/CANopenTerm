# Hello, world.

print_heading("Hello, world.")
nmt_send_command(0x00, 0x01, True)

while not key_is_hit():
    delay_ms(10)
