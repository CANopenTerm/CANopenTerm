# Getting started

## Supported hardware

CANopenTerm is built on top of
[CANvenient](https://canopenterm.de/canvenient), which provides support
for a wide variety of CAN hardware, including USB-to-CAN adapters from
multiple manufacturers.  On Linux systems, CANvenient leverages SocketCAN,
enabling compatibility with any CAN hardware that has a SocketCAN driver
available.

## Download

A pre-compiled Windows installation package can be found on the
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

    SCRIPT            Run script, implies -p
                      Can't be combined with other options

    -s SCRIPT         Run script (.lua can be ommited)
    -i INTERFACE      Set CAN interface
    -b BAUD           Set baud rate
                        1 = 1 MBit/s
                        2 = 800 kBit/s
                        3 = 500 kBit/s
                        4 = 250 kBit/s
                        5 = 125 kBit/s
    -n NODE_ID        Set node ID, default: 0x01
    -f                Full screen widget window
    -p                Run in plain mode
    -t                Run conformance test, implies -p
```

Setting the baud rate on Linux has no effect, as the CAN bus is
configured via the `ip` command.

## Compilation & Installation

First clone the repository:
```bash
git clone https://github.com/CANopenTerm/CANopenTerm.git
```

### Windows

The easiest way to get CANopenTerm up and running is Visual Studio 2026
with [C++ CMake tools for
Windows](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
installed.  Simply open the cloned repository via `File -> Open ->
Folder`.  All required dependencies are fetched at compile time.

### Debian

![CANopenTerm](media/diversity-2019.svg ':no-zoom')

CANopenTerm and CANvenient will soon be included in Debian. If you’re
using the `unstable` branch, you'll be able to install them easily with:

```bash
sudo apt install canopenterm
```

The Debian packages are maintained by
[Christopher Obbard](mailto:obbardc@debian.org).

The Debian diversity logo was created by
[Valessio Brito](https://gitlab.com/valessiobrito/artwork) and it's
licensed under GPLv3.

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
```

### System Libraries

When system-provided libraries are available (e.g. via a distribution
package manager or an embedded Linux build system), pass
`-DUSE_SYSTEM_LIBS=ON` to CMake to skip fetching dependencies at build
time and link against the installed libraries instead.
