# Getting started

## Supported hardware

CANopenTerm is built on top of
[CANvenient](https://canopenterm.de/canvenient), which provides support
for a wide variety of CAN hardware, including USB-to-CAN adapters from
multiple manufacturers.  On Linux systems, CANvenient leverages SocketCAN,
enabling compatibility with any CAN hardware that has a SocketCAN driver
available.

## Download & Installation

### Windows

#### Microsoft Store (Recommended)

The easiest way to install CANopenTerm on Windows is through
the Microsoft Store:

<a href="https://get.microsoft.com/installer/download/9N1DQ5X8CL6Z?referrer=appbadge" target="_self" >
    <img src="https://get.microsoft.com/images/en-us%20light.svg" width="300"/>
</a>

The Microsoft Store version automatically receives updates and
includes all necessary dependencies.

#### Standalone Installer

If you prefer not to use the Microsoft Store, standalone packages
are available on the
[Releases page on GitHub](https://github.com/CANopenTerm/CANopenTerm/releases/latest).

Download either:
- **Portable ZIP package** - Extract and run without installation
- **MSI installer** - Traditional Windows installer with desktop shortcut

### Linux

#### Debian/Ubuntu

![CANopenTerm](media/diversity-2019.svg ':no-zoom')

CANopenTerm is available in Debian. If you're using the `testing`
branch, you can install it easily with:

```bash
sudo apt install canopenterm
```

For other Debian-based distributions, you may need to build from
source (see below).

The Debian package is maintained by
[Christopher Obbard](mailto:obbardc@debian.org).

The Debian diversity logo was created by
[Valessio Brito](https://gitlab.com/valessiobrito/artwork) and it's
licensed under GPLv3.

#### Yocto/OpenEmbedded

![CANopenTerm](media/oe-logo-2009.svg ':no-zoom')

CANopenTerm is included in the
[OpenEmbedded](https://www.openembedded.org)/[Yocto project](https://www.yoctoproject.org/).
The recipe can be found at
[meta-oe/recipes-extended/canopenterm](https://layers.openembedded.org/layerindex/recipe/421300/).

To build CANopenTerm in your Yocto environment:

```bash
cd ~/poky
source oe-init-build-env
bitbake canopenterm
```

## Building from Source

### Prerequisites

For Linux systems, install the required development libraries:

```bash
sudo apt-get install libreadline-dev libsocketcan-dev cmake build-essential
```

### Build Instructions

```bash
git clone https://github.com/CANopenTerm/CANopenTerm.git
cd CANopenTerm
mkdir build
cd build
cmake ..
make
```

### Using System Libraries

By default, CANopenTerm fetches its dependencies at build time.
If you prefer to use system-provided libraries (e.g., from a
package manager or embedded Linux build system), pass
`-DUSE_SYSTEM_LIBS=ON` to CMake:

```bash
cmake -DUSE_SYSTEM_LIBS=ON ..
make
```

This option is useful for distribution packaging or when
building for embedded systems.
