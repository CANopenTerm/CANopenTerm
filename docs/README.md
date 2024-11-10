# CANopenTerm

[![CANopenTerm](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/media/logo.svg)](https://raw.githubusercontent.com/CANopenTerm/CANopenTerm/main/media/logo.svg?raw=true "CANopenTerm")

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0b16a90be6d4a59beafcabd727b2a2f)](https://app.codacy.com/gh/CANopenTerm/CANopenTerm/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Linux Build & Unit Tests](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/linux.yml/badge.svg)](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/linux.yml)
[![Windows Build](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/windows.yml/badge.svg)](https://github.com/CANopenTerm/CANopenTerm/actions/workflows/windows.yml)

## Overview

CANopenTerm is an open-source software tool designed for the
development, testing, and analysis of CANopen CC networks and
devices. It extends its capabilities to support other CAN CC
protocols, including SAE J1939 and OBD-II, making it a
versatile tool for professionals working with CAN CC-based
networks.

Key functionalities include:

- **Monitoring and Analysis**:
  Advanced trace filtering and node access for comprehensive
  network monitoring and analysis.

- **Configuration and Simulation**:
  Tools for configuring CANopen CC devices and simulating nodes
  under development.

- **Testing**:
  Capabilities to thoroughly test CANopen CC networks and devices.

- **Extended Protocol Support**:
  Features tailored for CANopen, SAE J1939, and OBD-II protocols.

## Key Features

- **Read and Write SDOs**:
  Manage Service Data Objects with a user-friendly interface.

- **Send NMT Messages**:
  Control network management with commands to one or all nodes.

- **Simulate PDOs**:
  Add asynchronous Process Data Objects to simulate network
  traffic.

- **Versatile Raw CAN CC Interface**:
  For non-CANopen needs, offering flexibility and control.

- **Workflow Automation**:
  Automate your workflow with pre-included scripts, such as a
  [PEAK CAN trace](https://www.peak-system.com/produktcd/Pdf/English/PEAK_CAN_TRC_File_Format.pdf)
  record and playback script.  Additionally, you have the flexibility to write
  your own scripts in either [Lua](https://canopenterm.de/lua-api) or
  [PicoC](https://canopenterm.de/picoc-api) to customize and automate tasks.
  Both languages are supported by similar APIs, offering equal functionality
  to cater to your automation needs.

- **Built-in DBC Parser**:
  Import and utilize
  [DBC files](https://www.csselectronics.com/pages/can-dbc-file-database-intro)
  for streamlined CAN CC network projects.

- **Conformance Testing**:
  Verify the object dictionary of your CANopen CC devices with the built-in
  electronic data sheet (EDS) parser.

## Code Quality and Standards

- **ANSI C Source Code**:
  Ensures robust performance and high portability.

- **Quality Assurance**:
  Static code analysis and unit tests for reliability.

- **OS Abstraction Layer**:
  Simplifies the process of porting to different platforms.

## Open Source Commitment

CANopenTerm is developed as an open-source project, freely
available for use and contribution by the community. It aims
to provide a technical, no-nonsense tool for professionals
working with CAN CC-based networks, offering depth and versatility
in its features without the need for marketing embellishments.

## Why Your Support Matters

**CANopenTerm** is developed to provide a powerful tool for
professionals at no cost. However, maintaining and improving
such a project requires resources. Your donations can make a
significant difference, enabling continued development and support.

Your contribution, no matter how small, will be greatly appreciated
and will help keep CANopenTerm thriving. Together, we can ensure
this essential tool remains available and continues to evolve.

[Donate Now](https://github.com/sponsors/mupfdev)
