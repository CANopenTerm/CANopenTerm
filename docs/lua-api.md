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

```lua
dbc_decode (can_id, [data])
```

> **can_id** CAN-ID.

> **data** Data, default is `0x0000000000000000`.

**Returns**: Decoded output as a string.

```lua
dbc_load (filename)
```

> **filename** DBC file name.

**Returns**: true on success, false on failure.

## Network management (NMT)

### nmd_send_command()

```lua
nmd_send_command (node_id, command, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **command** NMT command code.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: true on success, false on failure.

!> The **command** parameter supports the following commands:

| Command | Description                    |
| ------- | ------------------------------ |
| 0x01    | Start (go to Operational)      |
| 0x02    | Stop (go to Stopped)           |
| 0x80    | Go to Pre-operational          |
| 0x81    | Reset node (Application reset) |
| 0x82    | Reset communication            |

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

```lua
pdo_add (can_id, event_time_ms, length, [data], [show_output], [comment])
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Data length in bytes.

> **data** Data, default is `0x0000000000000000`.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: true on success, false on failure.

### pdo_del()

```lua
pdo_del (can_id)
```

> **can_id** CAN-ID.

**Returns**: true on success, false on failure.

## Service data objects (SDO)

### sdo_lookup_abort_code()

```lua
sdo_lookup_abort_code (abort_code)
```

> **abort_code** SDO abort code.

**Returns**: Abort code description (string).

### sdo_read()

Read SDO (expedided or segmented).

```lua
sdo_read (node_id, index, sub_index, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output.
              If ommited, the description from the CANopen object dictionary is used.

**Returns**: number or string, nil on failure.

### sdo_write()

Write expedided SDO.

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

**Returns**: true on success, false on failure.

### sdo_write_file()

Write file (block transfer).

```lua
sdo_write_file (node_id, index, sub_index, filename)
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **filename** The name of the file to be sent.

**Returns**: true on success, false on failure.

### sdo_write_string()

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

**Returns**: true on success, false on failure.

### dict_lookup()

Read CANopenTerm from built-in object directory.

```lua
dict_lookup (index, sub_index)
```

> **index** Index.

> **sub_index** Sub-Index.

**Returns**: a string or nil.

## Generic CAN interface

### can_write()

### can_read()

```lua
can_read ()
```

**Returns**: id, length, data and timestamp in μs, or nil on failure.

```lua
can_write (can_id, data_length, [data], [show_output], [comment])
```

> **can_id** CAN-ID.

> **length** Data length in bytes.

> **data** Data, default is `0x0000000000000000`.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: true on success, false on failure.

## Program flow

### delay_ms

```lua
delay_ms ([delay_in_ms], [show_output], [comment])
```

> **delay_in_ms** Delay in milliseconds, default is 1000.

> **show_output** Show formatted output, default is `false`.

> **comment** Comment to show in formatted output, default is `nil`.

**Returns**: Nothing.

### key_is_hit()

```lua
key_is_hit ()
```

**Returns**: true or false.

## Miscellaneous

### print_heading()

```lua
print_heading (heading)
```

> Heading to be printed.

**Returns**: Nothing.

### Helper functions

In addition to the integrated API, there are also a couple of helper functions that can be imported:

```lua
local canopen = require "lua/canopen"
local utils   = require "lua/utils"

local node_list, total_devices = canopen.find_devices(timeout_ms)
local node_id   = canopen.get_id_by_name("Device name")
local hex_files = utils.get_file_list("hex")
```
