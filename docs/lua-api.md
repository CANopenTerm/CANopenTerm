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

> **data** Data, default is `0`.

**Returns**: Decoded output as a string.

<!-- tab:Example -->
```lua
local utils    = require "lua/utils"
local watch_id = 0x18F01DFE -- Steer Angle Sensor

if false == dbc_load("dbc/j1939.dbc") then
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

if dbc_load("dbc/j1939.dbc") then
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
if false == dbc_load("dbc/j1939.dbc") then
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
nmt_send_command(0x00, 0x81) -- Reset.

while false == key_is_hit() do
  local id, length, message = can_read()

  if length == 1 and message == 0x00 -- Bootup message.
  then
    print("Node back online.")
    break
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

> **data** Data, default is `0`.

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
if pdo_del(0x181, true, "TPDO1") then
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

> **data** Data, default is `0`.

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
Read from built-in object directory.

```lua
dict_lookup (index, sub_index)
```

<!-- tab:Example -->
```lua
print(dict_lookup(0x1008, 0x00)) -- Manufacturer device name.
```
<!-- tabs:end -->

> **index** Index.

> **sub_index** Sub-Index.

**Returns**: a string or `nil`.

## Test Report Generation

### test_add_result()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
test_add_result ([has_passed], [time], [package], [class_name], [test_name], [error_type], [error_message], [call_stack])
```

> **has_passed** Test result, default is `false`.

> **time** Test execution time in seconds, default is `0.0`.

> **package** Package name, default is `Tests`.

> **class_name** Class name, default is `Generic`.

> **test_name** Test name, default is `UnnamedTest`.

> **error_type** Error type, default is `AssertionError`.

> **error_message** Error message, default is `No error message provided`.

> **call_stack** Call stack, default is `<!-- No call stack provided. -->`.

**Returns**: Nothing.

<!-- tab:Example -->
```lua
test_add_result(true,  1.22, "Tests", "Registration", "testCase1")
test_add_result(true,  2.34, "Tests", "Registration", "testCase2")
test_add_result(true,  2.52, "Tests", "Auth",         "testCase1")
test_add_result(false, 4.34, "Tests", "Registration", "testCase3")
```
<!-- tabs:end -->

### test_clear_results()

<!-- tabs:start -->
<!-- tab:Description -->
!> Implicitly called by `test_generate_report()`

```lua
test_clear_results ()
```

**Returns**: Nothing.

<!-- tab:Example -->
```lua
test_add_result(true, 1.22, "Tests", "Registration", "testCase1")
test_clear_results()
test_generate_report()
```

Resulting XML file:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites time="0.000000">
</testsuites>
```
<!-- tabs:end -->

### test_eds_file()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
test_eds_file (node_id, file_name, [package])
```

> **node_id** CANopen Node-ID.

> **file_name** EDS file name.

> **package** Package name, "EDS" if ommited.

**Returns**: Nothing.

<!-- tab:Example -->
```lua
test_eds_file(0x50, "eds/DS301_profile.eds")
test_generate_report()
```
<!-- tabs:end -->

### test_generate_report()

<!-- tabs:start -->
<!-- tab:Description -->
!> Implicitly calls `test_clear_results()`

```lua
test_generate_report ([file_name])
```

> **file_name** Test report output file name, default is `test_report.xml`.

**Returns**: `true` on success, `false` on failure.

<!-- tab:Example -->
```lua
test_add_result(true,  1.22, "Tests", "Registration", "testCase1")
test_add_result(true,  2.34, "Tests", "Registration", "testCase2")
test_add_result(true,  2.52, "Tests", "Auth",         "testCase1")
test_add_result(false, 4.34, "Tests", "Registration", "testCase3")
test_generate_report()
```

Resulting XML file:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<testsuites time="10.420000">
    <testsuite name="Tests.Auth" time="2.520000">
        <testcase name="testCase1" classname="Tests.Auth" time="2.520000">
        </testcase>
    </testsuite>
    <testsuite name="Tests.Registration" time="7.900000">
        <testcase name="testCase2" classname="Tests.Registration" time="2.340000">
        </testcase>
        <testcase name="testCase1" classname="Tests.Registration" time="1.220000">
        </testcase>
        <testcase name="testCase3" classname="Tests.Registration" time="4.340000">
            <failure message="No error message provided" type="AssertionError">
                <!-- No call stack provided. -->
            </failure>
        </testcase>
    </testsuite>
</testsuites>
```
<!-- tabs:end -->

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

> **data** Data, default is `0`.

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

### can_flush()

<!-- tabs:start -->
<!-- tab:Description -->
Flush receive and transmit queues.

```lua
can_flush ()
```

<!-- tab:Example -->
```lua
can_flush()
```
<!-- tabs:end -->

**Returns**: Nothing.

### dict_lookup_raw()

<!-- tabs:start -->
<!-- tab:Description -->
Interpret raw CAN message.

```lua
dict_lookup_raw (can_id, data_length, [data])
```

> **can_id** CAN-ID.

> **length** Data length in bytes.

> **data** Data, default is `0`.

<!-- tab:Example -->
```lua
print(dict_lookup_raw(0x730, 1, 0x05)) -- Heartbeat: Operational.
```
<!-- tabs:end -->

## Miscellaneous

### delay_ms()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
delay_ms ([delay_in_ms], [show_output], [comment])
```

> **delay_in_ms** Delay in milliseconds, default is 1.

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

> **heading** Heading to be printed.

**Returns**: Nothing.

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

## Widgets

### window_clear()

<!-- tabs:start -->
<!-- tab:Description -->
Clear widget window.

```lua
window_clear ()
```
**Returns**: Nothing.

<!-- tabs:start -->
<!-- tab:Description -->
Hide widget window.

```lua
window_hide ()
```
**Returns**: Nothing.

### window_show()

<!-- tabs:start -->
<!-- tab:Description -->
Show widget window.

```lua
window_show ()
```

**Returns**: Nothing.

### window_update()

<!-- tabs:start -->
<!-- tab:Description -->
Update widget window.

```lua
window_update ()
```

**Returns**: Nothing.

### widget_tachometer()

<!-- tabs:start -->
<!-- tab:Description -->
Draw tachometer-like widget.

```lua
widget_tachometer (pos_x, pos_y, size, max, rpm)
```

> **pos_x** Horizontal position on widget window.

> **pos_y** Vertical position on widget window.

> **size** Size in pixel.

> **max** Maximum value.

> **rpm** Set value.

**Returns**: Nothing.

## Utilities

<!-- tabs:start -->
<!-- tab:Description -->
In addition to the integrated API, there are also a couple of utility
functions that can be imported.

<!-- tab:Example -->
```lua
local utils      = require "utils"
local hex_files  = utils.get_file_list("hex")
local temp, time = utils.measure_time(function() return delay_ms(2) end)
print(string.format("Elapsed time: %.2f milliseconds.", time))
```
<!-- tabs:end -->
