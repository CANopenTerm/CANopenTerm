# Lua API

One of the essential and certainly most useful parts of CANopenTerm is
the possibility to write scripts to automate recurring processes or
tests.

To write your own scripts, please refer to the official Lua 5.4
reference manual which can be found here: [Lua 5.4 Reference
Manual](https://www.lua.org/manual/5.4/).

Scripts located in the `scripts` subdirectory will be detected
automatically.

In addition to the standard functions and basic features of the Lua
programming language, CANopenTerm also provides its own functions.
These are explained in detail here.

## Network management (NMT)

NMT commands can be sent with the following Lua function:

```lua
nmt_send_command (node_id, nmt_command)
```

**Returns**: true on success, false on failure.

The following commands are supported:

```text
0x01 = Start (go to Operational)
0x02 = Stop (go to Stopped)
0x80 = Go to Pre-operational
0x81 = Reset node (Application reset)
0x82 = Reset communication
```

## Process data objects (PDO)

It is possible to create up to 632 asynchronous PDOs, which are then
sent cyclically at the specified interval.  This is extremely useful,
especially for testing purposes.

The following functions are available for this task:

```lua
pdo_add (can_id, event_time_ms, length, data_d0_d3, data_d4_d7)
pdo_del (can_id)
```

The CAN-IDs reserved according to CiA 301 can be used:

```text
0x000 - 0x07f (Node-ID)
0x181 - 0x1ff (TPDO1)
0x281 - 0x2ff (TPDO2)
0x381 - 0x3ff (TPDO3)
0x481 - 0x4ff (TPDO4)
```

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
can_write (can_id, data_length, data_d0_d3, data_d4_d7)
```

**Returns**: true on success, false on failure.

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
while (condition)
do
    if (key_is_hit())
    then
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
