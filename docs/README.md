![CANopenTerm](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/media/logo.svg)

# CANopenTerm

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0b16a90be6d4a59beafcabd727b2a2f)](https://app.codacy.com/gh/CANopenTerm/CANopenTerm/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![CMake](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/cmake.yml/badge.svg)](https://github.com/mupfdev/CANopenTerm/actions/workflows/cmake.yml)
[![Microsoft C++ Code Analysis](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/msvc.yml/badge.svg)](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/msvc.yml)

## Description

Do you work in an R&D environment and are you responsible for
developing or testing CANopen devices? Are you an application
engineer looking for a simple tool to communicate easily and
efficiently on a CANopen network?

Then you are in the right place!

CANopenTerm is a versatile yet easy-to-use software tool for
analysing and testing CANopen networks and devices using an
interactive terminal. 
addition to reading and writing **Service Data Objects**
(**SDO**) and sending **Network Management Commands**
(**NMT**), it is also possible to simulate asynchronous
**Process Data Objects** (**PDO**).

Complex, recurring tasks can be automated using scripts
written in [Lua 5.4](https://www.lua.org/manual/5.4/) and a
[custom API](https://canopenterm.de/lua-api).

In addition, CANopenTerm is completely free and has been
released under the terms of a
[permissive open source licence](https://github.com/CANopenTerm/CANopenTerm/blob/main/LICENSE.md).

## Features

- Service Data Objects (**SDO**)  
  Write to nodes using expedited and segmented SDOs in a in a
  user-friendly way.  
  Built-in object dictionary.

 - Network Management (**NMT**)  
   Send network management (**NMT**) messages to one or all
   nodes.

- Process Data Objects (**PDO**)  
  To simulate periodic traffic on the network, asynchronous PDOs
  can be easily added.

- Lua 5.4 Interpreter  
  Automate your workflow by writing Scripts in
  [Lua 5.4](https://www.lua.org/manual/5.4/).  
  [Custom API](https://canopenterm.de/lua-api) including a raw CAN interface.

- Highly portable and cross-platform  
  CANopenTerm is written entirely in ANSI C and has very few dependencies such
  as [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) and
  [Lua 5.4](https://www.lua.org/about.html).  
  [SocketCAN](https://www.kernel.org/doc/html/latest/networking/can.html)
  support on Linux.  
  [PEAK CAN](https://www.peak-system.com/) interface support
  on Windows.  

## Support

CANopenTerm is a free and open source project from which I do not make
any money.  Do you still want to support the project? Then please
consider making a small donation, which would be greatly appreciated!

[![Donate](https://canopenterm.de/media/de-pp-logo-200px.png)](https://www.paypal.com/donate/?hosted_button_id=9K6YZYKPNUN9Q "Donate")

## Screenshot

[![CANopenTerm screenshot](https://canopenterm.de/media/screenshot.png)](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/docs/media/screenshot.png?raw=true "CANopenTerm screenshot")
