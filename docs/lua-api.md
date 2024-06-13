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
pdo_add (can_id, event_time_ms, length, data_d0_d3, data_d4_d7)
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Message length in bytes.

> **data_d0_d3** Data byte 0 to 3.

> **data_d4_d7** Data byte 4 to 7.

### pdo_del()

```lua
pdo_del (can_id)
```

> **can_id** CAN-ID.

## Service data objects (SDO)

To read service data objects (SDO):

```lua
sdo_read (node_id, index, sub_index, show_output = false, comment = nil)
```

**Returns**: number or string, nil on failure.

```lua
print (sdo_read (0x50, 0x2100, 1))
sdo_write (0x50, 0x2100, 1, 4, 1)
print (sdo_read (0x50, 0x2100, 1))
```

To write SDOs, the following function is available:

```lua
sdo_write (node_id, index, sub_index, length, data, show_output = false, comment = nil)
```

**Returns**: true on success, false on failure.

CANopenTerm has a built-in object directory. A description can be looked-up
up using the following function:

```lua
dict_lookup (index, sub_index)
```

**Returns**: a string or nil.

## Generic CAN interface

In addition, there are also functions to address the CAN directly:

```lua
can_write (can_id, data_length, data_d0_d3, data_d4_d7, show_output = false, comment = nil)
```

**Returns**: true on success, false on failure.

Reading CAN messages:

```lua
local id, length, data = can_read ()
```

**Returns** id, length and data (as string), or nil on failure.

## Program flow

Lua does not provide its own function to delay the program flow.  The
following function is made available for this purpose:

```lua
delay_ms (delay_in_ms)
```

The is_key_hit() function can be used to check whether a key has been pressed:

```lua
key_is_hit ()
```

**Returns**: true or false.

For example, loops can be interrupted as follows:

```lua
while (condition) do
    if (key_is_hit()) then
        break
    end
end
```

## Miscellaneous

If you enable the optional output, it may be beneficial to have a meaningful
heading:

```lua
local index     = 0x1018
local sub_index = 2

print_heading("What am I?")
sdo_read( 0x40, index, sub_index_, true, dict_lookup( index, sub_index) )
```
