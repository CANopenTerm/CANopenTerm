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
sdo_read (node_id, index, sub_index)
```

The result is not displayed automatically, but the most recent result
can be accessed via the global variable `sdo_result` which can be reset to
`0` using the function `sdo_reset_result()`:

```lua
sdo_read (0x50, 0x2100, 1)
print(sdo_result)
sdo_write (0x50, 0x2100, 1, 4, 1)
sdo_read (0x50, 0x2100, 1)
print(sdo_result)
sdo_reset_result()
```

To write SDOs, the following function is available:

```lua
sdo_write (node_id, index, sub_index, length, data)
```

## Generic CAN interface

In addition, there are also functions to address the CAN directly:

```lua
can_write (can_id, data_length, data_d0_d3, data_d4_d7)
```

## Program flow

Lua does not provide its own function to delay the program flow.  The
following function is made available for this purpose:

```lua
delay_ms (delay_in_ms)
```

For example, loops can be interrupted as follows:

```lua
while (condition)
do
    poll_keys()
    if (key_is_hit)
    then
        break
    end
end
```
