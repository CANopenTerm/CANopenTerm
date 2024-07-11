# CANopenTerm

[![CANopenTerm](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/media/logo.svg)](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/media/logo.svg?raw=true "CANopenTerm")

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0b16a90be6d4a59beafcabd727b2a2f)](https://app.codacy.com/gh/CANopenTerm/CANopenTerm/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Linux Build & Unit Tests](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/linux.yml/badge.svg)](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/linux.yml)
[![Windows Build](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/windows.yml/badge.svg)](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/windows.yml)

## Discover CANopenTerm

**Are you an R&D professional or application engineer working
with CAN & CANopen networks? Do you need a robust, easy-to-use
tool for developing or testing CANopen nodes?**

Look no further!

[![CANopenTerm](https://canopenterm.de/media/CANopenTerm.gif)](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/docs/media/CANopenTerm.gif?raw=true "CANopenTerm")

CANopenTerm is your go-to software for analyzing and testing
CANopen nodes with ease.  Empower your R&D with CANopenTerm,
the interactive terminal tool that simplifies communication
on a CANopen network, allowing you to:

- **Read and Write SDOs**  
  Easily manage Service Data Objects with a user-friendly
  interface and built-in object dictionary.

- **Send NMT Messages**  
  Control network management with commands to one or all nodes.

- **Simulate PDOs**  
  Effortlessly add asynchronous Process Data Objects to mimic
  network traffic.

- **Versatile Raw CAN Interface**  
  Unlock the full potential of your CAN network with CANopenTerm's
  versatile raw CAN interface.  For all your non-CANopen needs,
  this robust feature offers unparalleled flexibility and control.

- **Automate your Workflow**  
  Use Lua scripts to automate your workflow.  A variety of
  scripts included, such as a
  [PEAK CAN trace](https://www.peak-system.com/produktcd/Pdf/English/PEAK_CAN_TRC_File_Format.pdf)
  record and playback script.

## Key Features

- **Lua 5.4 Interpreter**  
  Write scripts to automate tasks and leverage a [custom API](/lua-api).

- **Built-in DBC (CAN Database File) Parser**  
  Seamlessly import and utilize
  [DBC files](https://www.csselectronics.com/pages/can-dbc-file-database-intro)
  to streamline your CAN network projects. This feature enhances your
  ability to work with various protocols, including OBD-II for vehicle
  diagnostics and J1939 for heavy-duty vehicle networking, making it
  easier to manage and interpret CAN network data.

  [![DBC Monitor](https://canopenterm.de/media/DBC_Monitor.gif)](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/docs/media/DBC_Monitor.gif?raw=true "DBC Monitor")
 
- **Robust and Portable Source Code**  
  - CANopenTerm's source code is written entirely in ANSI C, ensuring
    robust performance and high portability with minimal dependencies.
  - The quality of the code is constantly monitored through static code
    analysis and an ever-growing number of unit tests, guaranteeing
    reliability and maintainability.
  - The OS abstraction layer makes it easy to port the program to other
    platforms, further enhancing its versatility.

- **Open Source and Free**  
  Released under a permissive open-source license, CANopenTerm is
  free for all to use.

## Why Your Support Matters

**CANopenTerm** is developed to provide a powerful tool for professionals
at no cost.  However, maintaining and improving such a project requires
resources.  Your donations can make a significant difference, enabling
continued development and support.

**Join Us in Advancing CANopen Development!**

Your contribution, no matter how small, will be greatly appreciated and
will help keep CANopenTerm thriving. Together, we can ensure this essential
tool remains available and continues to evolve.

Thank you for your support!

[![Buy Me A Coffee](https://canopenterm.de/media/buy-me-a-coffee.png)](https://www.buymeacoffee.com/mupf.dev)

## Documentation

The documentation can be found [here](https://canopenterm.de).

## License

This project is licensed under the "The MIT License".  See the file
[LICENSE.md](LICENSE.md) for details.

J1939 DBC from
[Haskell.org](https://hackage.haskell.org/package/ecu-0.0.8/src/src/j1939_utf8.dbc),
original author unknown.

CANopen® is a trademark of the CAN in Automation User's Group
