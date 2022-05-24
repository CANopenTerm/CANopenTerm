print "Enter node ID:"
node_id = io.read("*number")

io.write("Device type: ")
io.flush()
read_sdo(node_id, 0x1000, 0)
print("")
