# Getting started

## Supported hardware

On Windows, only CAN hardware from [PEAK-System Technik
GmbH](https://www.peak-system.com/Products.57.0.html?L=1) is currently
supported.  SocketCAN is used on Linux, so the CAN can be accessed
hardware-independently.

## Download

A pre-compiled Windows version of CANopenTerm can be found on the
[Releases page on GitHub](https://github.com/CANopenTerm/CANopenTerm/releases/latest).

## Usage

### Command-line interface

CANopenTerm is an interactive terminal which can be used via a number of
commands.  You can get a detailed overview of all available command by
entering `h`.

### Run script from command-line

It is possible to run or set the CAN interface from the command line.

Usage: `CANopenTerm [-s script] [-i can_interface]`

The `.lua` file extension can be omitted.  
The `.c` file extension has to be specified.

#### Linux

```bash
./CANopenTerm -s hello -i can1
```

#### Windows

```bash
CANopenTerm.exe -s hello
```

## Compiling

First clone the repository:
```bash
git clone https://github.com/CANopenTerm/CANopenTerm.git
```

### Windows

The easiest way to get CANopenTerm up and running is Visual Studio 2022
with [C++ CMake tools for
Windows](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
installed.  Simply open the cloned repository via `File -> Open ->
Folder`.  All required dependencies are fetched at compile time.

### Linux

CANopenTerm can also be compiled on Linux with the included CMake
configuration.  

```bash
sudo apt-get install libreadline-dev libsocketcan-dev
mkdir build
cd build
cmake ..
make
````

### Raspberry Pi

To compile CANopenTerm on the Raspberry Pi, another dependency is required.

```bash
sudo apt-get install libreadline-dev libsocketcan-dev libraspberrypi-dev
mkdir build
cd build
cmake ..
make
````

### Yocto

CANopenTerm also provides an experimental
[Yocto layer](https://github.com/CANopenTerm/meta-canopenterm).

```bash
bitbake-layers add-layer meta-canopenterm
bitbake canopenterm
````
