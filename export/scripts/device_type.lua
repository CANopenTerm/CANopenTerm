io.write("Device type: ")
io.flush()
read_sdo(0x50, 0x1000, 0)
print("")
