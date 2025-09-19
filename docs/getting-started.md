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

### Command-line arguments

It is possible to use a set of command-line arguments:

```
$ ./CANopenTerm -h

Usage: CANopenTerm [OPTION]

    -s SCRIPT         Run script
    -t EDS            Run EDS conformance test (implies -p)
    -i INTERFACE      Set CAN interface
    -b BAUD           Set baud rate
                        0 = 1 MBit/s
                        2 = 500 kBit/s
                        3 = 250 kBit/s
                        4 = 125 kBit/s
    -n NODE_ID        Set node ID, default: 0x01
    -p                Run in plain mode
```

Setting the baud rate on Linux has no effect, as the CAN bus is
configured via the `ip` command.

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

### Yocto

![CANopenTerm](media/oe-logo-2009.svg ':no-zoom')

CANopenTerm is upstreamed to the
[OpenEmbedded](https://www.openembedded.org)/[Yocto project](https://www.yoctoproject.org/).
The recipe can be found at
[meta-oe/recipes-extended/canopenterm](https://layers.openembedded.org/layerindex/recipe/421300/).

```bash
cd ~/poky
source oe-init-build-env
bitbake canopenterm
```

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
