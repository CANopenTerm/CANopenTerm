# Lua API

One of the essential and certainly most useful parts of CANopenTerm is
the possibility to write scripts to automate recurring processes.

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

## Service data objects (SDO)

To read service data objects (SDO):

```lua
sdo_read (node_id, index, sub_index)
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