# Getting started

## Supported hardware

Currently, only USB-based CAN dongles from [PEAK-System Technik
GmbH](https://www.peak-system.com/Products.57.0.html?L=1) are supported.
An abstraction for other manufacturers is being considered.

## Command-line interface

CANopenTerm is an interactive terminal which can be used via a number of
commands. You can get a detailed overview of all available command by
entering `h`.

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
