# Lua API

One of the most important and certainly most useful parts of
CANopenTerm is the integrated Lua parser, which can execute
scripts to automate recurring processes and tests.

To write your own, please refer to the official Lua 5.4
reference manual which can be found here: [Lua 5.4 Reference
Manual](https://www.lua.org/manual/5.4/).

In addition to the standard functions and basic features of the Lua
programming language, CANopenTerm also provides its own API, which is
documented in detail below.

## CAN Database File (DBC)

### dbc_decode()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
dbc_decode (can_id, [data])
```

> **can_id** CAN-ID.

> **data** Data, default is `0x0000000000000000`.

**Returns**: Decoded output as a string.

<!-- tab:Example -->
```lua
local utils    = require "lua/utils"
local watch_id = 0x18F01DFE -- Steer Angle Sensor

if false == dbc_load("j1939.dbc") then
  print("Failed to load DBC file.")
  return
end

local output    = dbc_decode(watch_id)
local num_lines = select(2, output:gsub('\n', '\n')) + 1

while false == key_is_hit() do
  local id, length, data = can_read()

  if id == watch_id then
    output = dbc_decode(watch_id, data)

    utils.print_multiline_at_same_position(output, num_lines)
  end
end
```
<!-- tabs:end -->

### dbc_find_id_by_name()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
dbc_find_id_by_name (search)
```

> **search** A case-insensitive substring to search within message names.

**Returns**: CAN-ID or `nil`.

<!-- tab:Example -->
```lua
local watch_id = 0

if dbc_load("j1939.dbc") then
  watch_id = dbc_find_id_by_name("sensor")
else
  print("Failed to load DBC file.")
  print("Exiting.")
  return
end

while false == key_is_hit() do
  local id, length, data = can_read()

  if id == watch_id then
    output = dbc_decode(watch_id, data)
    utils.print_multiline_at_same_position(output, num_lines)
  end
end
```
<!-- tabs:end -->

### dbc_load()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
dbc_load (filename)
```

> **filename** DBC file name.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if false == dbc_load("j1939.dbc") then
  print("Failed to load DBC file.")
end
```
<!-- tabs:end -->

## Network management (NMT)

!> The **command** parameter supports the following commands:

| Command | Description                    |
| ------- | ------------------------------ |
| 0x01    | Start (go to Operational)      |
| 0x02    | Stop (go to Stopped)           |
| 0x80    | Go to Pre-operational          |
| 0x81    | Reset node (Application reset) |
| 0x82    | Reset communication            |

### nmt_send_command()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
nmt_send_command (node_id, command, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **command** NMT command code.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
nmt_send_command(0x123, 0x81) -- Reset.

while false == key_is_hit() do
  local id, length, message = can_read()

  if length == 1 and message == 0x00 -- Bootup message.
  then
    print("Node back online.")
  end
end
```
<!-- tabs:end -->

## Process data objects (PDO)

It is possible to create up to 632 asynchronous PDOs, which are then
sent cyclically at the specified interval.

!>The following **CAN-IDs** can be used:

| From  | To    | Description |
| ----- | ----- | ----------- |
| 0x000 | 0x07f | Node-ID     |
| 0x181 | 0x1ff | TPDO1       |
| 0x281 | 0x2ff | TPDO2       |
| 0x381 | 0x3ff | TPDO3       |
| 0x481 | 0x4ff | TPDO4       |

### pdo_add()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
pdo_add (can_id, event_time_ms, length, [data], [show_output], [comment])
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Data length in bytes.

> **data** Data, default is `0x0000000000000000`.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if pdo_add(0x181, 100, 8, 0x1122334455667788, true, "TPDO1") then
  print("TPDO1 added.")
end
```
<!-- tabs:end -->

### pdo_del()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
pdo_del (can_id, [show_output], [comment])
```

> **can_id** CAN-ID.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if pdo_del(0x181) then
  print("TPDO1 deleted.")
end
```
<!-- tabs:end -->

## Service data objects (SDO)

### sdo_lookup_abort_code()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
sdo_lookup_abort_code (abort_code)
```

> **abort_code** SDO abort code.

**Returns**: Abort code description (string).

<!-- tab:Example -->
```lua
print(sdo_lookup_abort_code(0x05040005)) -- Out of memory.
```
<!-- tabs:end -->

### sdo_read()

<!-- tabs:start -->
<!-- tab:Description -->
Read SDO (expedited or segmented).

```lua
sdo_read (node_id, index, sub_index, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output.
              If ommited, the description from the CANopen object dictionary is used.

**Returns**:  

Expedited: number and `nil`, or number and string (if printable)  
Segmented: string and string  
On failure: `nil` and `nil`

<!-- tab:Example -->
```lua
local node_id        = 0x123
local temp, dev_name = sdo_read(node_id, 0x1008, 0x00)

if nil == dev_name then
  dev_name = "Unknown device"
end

print_heading(dev_name)
sdo_read(node_id, 0x1000, 0x00, true)
sdo_read(node_id, 0x1009, 0x00, true)
sdo_read(node_id, 0x100A, 0x00, true)
```
<!-- tabs:end -->

### sdo_write()

<!-- tabs:start -->
<!-- tab:Description -->
Write expedited SDO.

```lua
sdo_write (node_id, index, sub_index, length, [data], [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **length** Data length in bytes.

> **data** Data, default is `0x0000000000000000`.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if false == sdo_write(0x123, 0x1000, 0x00, 4, 0x12345678, true, "Device type") then
  print("Failed to write device type. Read-only object.")
end
```
<!-- tabs:end -->

### sdo_write_file()

<!-- tabs:start -->
<!-- tab:Description -->
Write file (block transfer).

```lua
sdo_write_file (node_id, index, sub_index, filename)
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **filename** The name of the file to be sent.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if sdo_write_string(0x123, 0x4500, 0x01, "Sup3rS3cuR3P4SSw0rd") then
  sdo_write_file(0x123, 0x4500, 0x05, "firmware.hex")
end
```
<!-- tabs:end -->

### sdo_write_string()

<!-- tabs:start -->
<!-- tab:Description -->
Write string (segmented transfer).

```lua
sdo_write_string (node_id, index, sub_index, "[data]", [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **data** The string to be written.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if sdo_write_string(0x123, 0x4600, 0x01, "Hello, world!") then
  if sdo_read(0x123, 0x4600, 0x01) == "Hello, world!" then
    print("All good!")
  end
end
```
<!-- tabs:end -->

### dict_lookup()

<!-- tabs:start -->
<!-- tab:Description -->
Read CANopenTerm from built-in object directory.

```lua
dict_lookup (index, sub_index)
```

<!-- tab:Example -->
```lua
print(dict_lookup, 0x1008, 0x00) -- Manufacturer device name.
```
<!-- tabs:end -->

> **index** Index.

> **sub_index** Sub-Index.

**Returns**: a string or `nil`.

## Generic CAN CC interface

### can_read()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
can_read ()
```

<!-- tab:Example -->
```lua
while key_is_hit() do
  local id, length, data, timestamp = can_read()

  print(string.format(
    "ID: 0x%03X, Length: %d, Data: 0x%016X, Timestamp: %d μs",
    id, length, data, timestamp))
end
```
<!-- tabs:end -->

**Returns**: id, length, data and timestamp in μs, or `nil` on failure.

### can_write()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
can_write (can_id, data_length, [data], [is_extended], [show_output], [comment])
```

> **can_id** CAN-ID.

> **length** Data length in bytes.

> **data** Data, default is `0x0000000000000000`.

> **is_extended** Extended frame, default is `false`.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
if can_write(0x5454, 8, 0x1122334455667788, false, true, "SPAM") then
  print("Message sent.")
end
```
<!-- tabs:end -->

## Miscellaneous

### delay_ms()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
delay_ms ([delay_in_ms], [show_output], [comment])
```

> **delay_in_ms** Delay in milliseconds, default is 1000.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: Nothing.

<!-- tab:Example -->
```lua
print("Waiting for 5 seconds.")
delay_ms(5000)
print("Done.")
```
<!-- tabs:end -->

### key_is_hit()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
key_is_hit ()
```

**Returns**: `true` or `false`.

<!-- tab:Example -->
```lua
while false == key_is_hit() do
  print("Waiting for key press.")
  delay_ms(100)
end
print("Exiting.")
```
<!-- tabs:end -->

### print_heading()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
print_heading (heading)
```

<!-- tab:Example -->
```lua
local device_name = sdo_read(0x123, 0x1008, 0x00)

if device_name then
  print_heading(device_name)
end
sdo_read(0x123, 0x1000, 0x00, true)
sdo_read(0x123, 0x1009, 0x00, true)
sdo_read(0x123, 0x100A, 0x00, true)
```
<!-- tabs:end -->

> Heading to be printed.

**Returns**: Nothing.

### Helper functions

<!-- tabs:start -->
<!-- tab:Description -->
In addition to the integrated API, there are also a couple of helper
functions that can be imported.

<!-- tab:Example -->
```lua
local canopen = require "lua/canopen"
local utils   = require "lua/utils"

local node_id   = canopen.get_id_by_name("Device name")
local hex_files = utils.get_file_list("hex")

local temp, time = utils.measure_time(function() return delay_ms(2) end)
print(string.format("Elapsed time: %.2f milliseconds.", time))
```
<!-- tabs:end -->
