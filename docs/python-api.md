# Python API

The Python API offers an alternative to the Lua scripting API.
It is based on pocketpy, a lightweight Python interpreter.
For a comprehensive overview of all features with respect to
cpython, please refer to the official documentation:
[pocketpy Features](https://pocketpy.dev/features/basic/).

## CAN Database File (DBC)

### dbc_decode()

<!-- tabs:start -->
<!-- tab:Description -->
```python
str dbc_decode (can_id, [data])
```

> **can_id** CAN-ID.

> **data** Data, default is `0`.

**Returns**: Decoded output as a `str`.

<!-- tab:Example -->
```python
import os

def print_multiline_at_same_position(message, num_lines):
    buffer = ""

    for _ in range(num_lines):
        buffer += chr(27) + "[2K"
        buffer += chr(27) + "[A"

    buffer += "\r"
    buffer += message

    print(buffer, end='')

if not dbc_load("dbc/j1939.dbc"):
    print("Failed to load DBC file")
else:
    watch_id  = 0x18F01DFE # Steer Angle Sensor
    output    = dbc_decode(watch_id)
    num_lines = len(output.split('\n'))

    os.system('cls')

    while not key_is_hit():
        result = can_read()

        if result:
            id   = result[0]
            data = result[2]

        if id == watch_id:
            output = dbc_decode(watch_id, data)
            print_multiline_at_same_position(output, num_lines)
```
<!-- tabs:end -->

### dbc_find_id_by_name()

<!-- tabs:start -->
<!-- tab:Description -->
```python
int dbc_find_id_by_name (search)
```

> **search** A case-insensitive sub`str` to search within message names.

**Returns**: CAN-ID or `None`.

<!-- tab:Example -->
```python
import os

def print_multiline_at_same_position(message, num_lines):
    buffer = ""

    for _ in range(num_lines):
        buffer += chr(27) + "[2K"
        buffer += chr(27) + "[A"

    buffer += "\r"
    buffer += message

    print(buffer, end='')

if not dbc_load("dbc/j1939.dbc"):
    print("Failed to load DBC file")
else:
    watch_id  = dbc_find_id_by_name("sensor")
    output    = dbc_decode(watch_id)
    num_lines = len(output.split('\n'))

    os.system('cls')

    while not key_is_hit():
        result = can_read()

        if result:
            id   = result[0]
            data = result[2]

        if id == watch_id:
            output = dbc_decode(watch_id, data)
            print_multiline_at_same_position(output, num_lines)
```
<!-- tabs:end -->

### dbc_load()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool dbc_load (filename)
```

> **filename** DBC file name.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if not dbc_load("dbc/j1939.dbc"):
    print("Failed to load DBC file")
else:
    print("Loaded DBC file")
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
```python
bool nmt_send_command (node_id, command, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **command** NMT command code.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output, default is `None`.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
while not key_is_hit():
  result = can_read()

  if result:
    length  = result[1]
    message = result[2]

  if length == 1 and message == 0x00: # Bootup message.
    print("Node back online.")
    break
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
```python
bool pdo_add (can_id, event_time_ms, length, [data], [show_output], [comment])
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if pdo_add(0x181, 100, 8, 0x1122334455667788, True, "TPDO1"):
  print("TPDO1 added.")
```
<!-- tabs:end -->

### pdo_del()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool pdo_del (can_id, [show_output], [comment])
```

> **can_id** CAN-ID.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output, default is `None`.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if pdo_del(0x181, True, "TPDO1"):
  print("TPDO1 deleted.")
```
<!-- tabs:end -->

## Service data objects (SDO)

### sdo_lookup_abort_code()

<!-- tabs:start -->
<!-- tab:Description -->
```python
str sdo_lookup_abort_code (abort_code)
```

> **abort_code** SDO abort code.

**Returns**: Abort code description (`str`).

<!-- tab:Example -->
```python
print(sdo_lookup_abort_code(0x05040005)) # Out of memory.
```
<!-- tabs:end -->

### sdo_read()

<!-- tabs:start -->
<!-- tab:Description -->
Read SDO (expedited or segmented).

```python
int/str sdo_read (node_id, index, sub_index, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**:  

Expedited: data (integer)
Segmented: data (string)
On failure: `None`

<!-- tab:Example -->
```python
node_id  = 0x123
result   = sdo_read(node_id, 0x1008, 0x00)
dev_name = result[1]

if None == dev_name:
  dev_name = "Unknown device"

print_heading(dev_name)
sdo_read(node_id, 0x1000, 0x00, True)
sdo_read(node_id, 0x1009, 0x00, True)
sdo_read(node_id, 0x100A, 0x00, True)
```
<!-- tabs:end -->

### sdo_write()

<!-- tabs:start -->
<!-- tab:Description -->
Write expedited SDO.

```python
bool sdo_write (node_id, index, sub_index, length, [data], [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if not sdo_write(0x123, 0x1000, 0x00, 4, 0x12345678, True, "Device type"):
  print("Failed to write device type. Read-only object.")
```
<!-- tabs:end -->

### sdo_write_file()

<!-- tabs:start -->
<!-- tab:Description -->
Write file (block transfer).

```python
bool sdo_write_file (node_id, index, sub_index, filename)
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **filename** The name of the file to be sent.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if sdo_write_string(0x123, 0x4500, 0x01, "Sup3rS3cuR3P4SSw0rd"):
  sdo_write_file(0x123, 0x4500, 0x05, "firmware.hex")
```
<!-- tabs:end -->

### sdo_write_string()

<!-- tabs:start -->
<!-- tab:Description -->
Write string (segmented transfer).

```python
bool sdo_write_string (node_id, index, sub_index, "[data]", [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **data** The `str` to be written.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if sdo_write_string(0x123, 0x4600, 0x01, "Hello, world!"):
  if sdo_read(0x123, 0x4600, 0x01) == "Hello, world!":
    print("All good!")
```
<!-- tabs:end -->

### dict_lookup()

<!-- tabs:start -->
<!-- tab:Description -->
Read CANopenTerm from built-in object directory.

```python
str dict_lookup (index, sub_index)
```

<!-- tab:Example -->
```python
print(dict_lookup(0x1008, 0x00)) # Manufacturer device name.
```
<!-- tabs:end -->

> **index** Index.

> **sub_index** Sub-Index.

**Returns**: a `str` or `None`.

## Generic CAN CC interface

### can_read()

<!-- tabs:start -->
<!-- tab:Description -->
```python
tuple can_read ()
```

<!-- tab:Example -->
```python
while not key_is_hit():
    result = can_read()
    if result:
        print(result)
```
<!-- tabs:end -->

**Returns**: (id, length, data, timestamp in μs), or `None` on failure.

### can_write()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool can_write (can_id, data_length, [data], [is_extended], [show_output], [comment])
```

> **can_id** CAN-ID.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **is_extended** Extended frame, default is `False`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
if can_write(0x5454, 8, 0x1122334455667788, False, True, "SPAM"):
    print("Message sent.")
```
<!-- tabs:end -->

## Miscellaneous

### delay_ms()

<!-- tabs:start -->
<!-- tab:Description -->
```python
None delay_ms ([delay_in_ms], [show_output], [comment])
```

> **delay_in_ms** Delay in milliseconds, default is 1000.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `None`.

<!-- tab:Example -->
```python
print("Waiting for 5 seconds.")
delay_ms(5000)
print("Done.")
```
<!-- tabs:end -->

### key_is_hit()

<!-- tabs:start -->
<!-- tab:Description -->
```python
key_is_hit ()
```

**Returns**: `True` or `False`.

<!-- tab:Example -->
```python
while not key_is_hit():
    print("Waiting for key press.")
    delay_ms(100)
print("Exiting.")
```
<!-- tabs:end -->

### print_heading()

<!-- tabs:start -->
<!-- tab:Description -->
```python
print_heading (heading)
```

> **heading** Heading to be printed.

<!-- tab:Example -->
```python
node_id  = 0x123
result   = sdo_read(node_id, 0x1008, 0x00)
dev_name = result[1]

if None == dev_name:
  dev_name = "Unknown device"

print_heading(dev_name)
sdo_read(node_id, 0x1000, 0x00, True)
sdo_read(node_id, 0x1009, 0x00, True)
sdo_read(node_id, 0x100A, 0x00, True)
```
<!-- tabs:end -->

**Returns**: `None`.
