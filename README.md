# CANopenTerm

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7a21b716448541289bb0b83b8bec7289)](https://www.codacy.com/gh/mupfdev/CANopenTerm/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mupfdev/CANopenTerm&amp;utm_campaign=Badge_Grade)
[![CMake](https://github.com/mupfdev/CANopenTerm/actions/workflows/cmake.yml/badge.svg)](https://github.com/mupfdev/CANopenTerm/actions/workflows/cmake.yml)

A versatile software tool to analyse and configure CANopen devices.

## Status

The project is currently at the early stages of its development.  It is
therefore not suitable for productive use.

## Supported hardware

Currently, only USB-based CAN dongles from [PEAK-System Technik
GmbH](https://www.peak-system.com/Products.57.0.html?L=1) are supported.
An abstraction for other manufacturers is being considered.

## Lua API

NMT commands can be sent with the following Lua function:

```lua
send_nmt_command (nmt_command, node_id)
```

The following commands are supported:

```
0x01 = Start (go to Operational)
0x02 = Stop (go to Stopped)
0x80 = Go to Pre-operational
0x81 = Reset node (Application reset)
0x82 = Reset communication
```

The node ID parameter must be between `0x00` and `0x7f`.

## Compiling

### Windows

The easiest way to get CANopenTerm up and running is Visual Studio 2022
with [C++ CMake tools for
Windows](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
installed.  Simply open the cloned repository via `File -> Open ->
Folder`.  Everything else is set up automatically.

### Linux

CANopenTerm can also be compiled on Linux with the included CMake
configuration.  All required dependencies are fetched at compile time.

```bash
mkdir build
cd build
cmake ..
make
````

## Licence and Credits

This project is licensed under the "The MIT License".  See the file
[LICENSE.md](LICENSE.md) for details.
