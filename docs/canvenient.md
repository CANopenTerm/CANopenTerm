# CANvenient

!> CANvenient is an abstraction layer for multiple CAN APIs on Windows.
It is currently under development and not yet released.

It can be found in its own repository:
[CANvenient on GitHub](https://github.com/CANopenTerm/CANvenient).

## API Reference

The API is intentionally designed to blend in with
[libsocketcan](https://git.pengutronix.de/cgit/tools/libsocketcan)
by Pengutronix, making it familiar for developers already working
with SocketCAN on Linux.

### can_find_interfaces()

<!-- tabs:start -->
<!-- tab:Description -->
Retrieves a list of available CAN interfaces on the system.

```c
int can_find_interfaces(struct can_iface* iface[], int* count)
```

> **iface** A pointer to an array of `can_iface` structures that will be populated with the details of the detected CAN interfaces.

> **count** A pointer to an integer that will be set to the number of CAN interfaces detected and stored in the `iface` array.

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
struct can_iface* iface[] = {0};
int count = 0;

if (0 == can_find_interfaces(iface, &count))
{
    for (int i = 0; i < count; i++)
    {
        struct can_iface* cur = &(*iface)[i];
        printf("[%d] %s ->", i, cur->name);
        cur->baudrate = CAN_BAUD_250K;

        if (0 == can_open(cur))
        {
            printf(" opened\n");
        }
        else
        {
            printf(" failed to open\n");
        }
    }
}

can_free_interfaces(iface, count);
```
<!-- tabs:end -->

### can_free_interfaces()

<!-- tabs:start -->
<!-- tab:Description -->
Deinitializes and frees the memory allocated for the CAN interfaces detected by `can_find_interfaces()`.

!> It is safe to call this function, even if `can_find_interfaces()` failed or
   returned `0` interfaces, as it will handle the cleanup gracefully.

```c
void can_free_interfaces(struct can_iface* iface[], int count);
```

> **iface** A pointer to an array of `can_iface` structures that will be populated with the details of the detected CAN interfaces.

> **count** A pointer to an integer that will be set to the number of CAN interfaces detected and stored in the `iface` array.

**Returns**: Nothing.

<!-- tab:Example -->
```c
struct can_iface* iface[] = {0};
int count = 0;

can_find_interfaces(iface, &count))
can_free_interfaces(iface, count);
```
<!-- tabs:end -->

### can_open()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open(struct can_iface* iface);
```

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_open_fd()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open_fd(struct can_iface* iface);
```

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_close()

<!-- tabs:start -->
<!-- tab:Description -->
```c
void can_close(struct can_iface* iface);
```

**Returns**: Nothing.

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_send()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_send(struct can_iface* iface, struct can_frame* frame);
```

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_recv()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_recv(struct can_iface* iface, struct can_frame* frame);
```

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

## Enumerations

### can_baudrate

> **CAN_BAUD_1M**        1 MBit/s

> **CAN_BAUD_800K**    800 kBit/s

> **CAN_BAUD_500K**    500 kBit/s

> **CAN_BAUD_250K**    250 kBit/s

> **CAN_BAUD_125K**    125 kBit/s

> **CAN_BAUD_100K**    100 kBit/s

> **CAN_BAUD_95K**  95.238 kBit/s

> **CAN_BAUD_83K**  83.333 kBit/s

> **CAN_BAUD_50K**      50 kBit/s

> **CAN_BAUD_47K**  47.619 kBit/s

> **CAN_BAUD_33K**  33.333 kBit/s

> **CAN_BAUD_20K**      20 kBit/s

> **CAN_BAUD_10K**      10 kBit/s

> **CAN_BAUD_5K**        5 kBit/s

## Structures

### can_iface

<!-- tabs:start -->
<!-- tab:Description -->

```c
struct can_iface
{
    u32 id;
    u8 opened;
    char* name;
    enum can_baudrate baudrate;
    enum can_vendor vendor;
    void* internal;
};
```

> **id** Unique identifier for the CAN interface.

> **opened** Indicates whether the CAN interface is currently opened (1) or closed (0).

> **name** Name of the CAN interface (e.g., "CAN0", "CAN1", etc.).

> **baudrate** The [baud rate](#can_baudrate) of the CAN interface.

> **vendor** The vendor of the CAN interface.

> **internal** Internal data used by the CAN API.

<!-- tabs:end -->

### can_frame

<!-- tabs:start -->
<!-- tab:Description -->

```c
struct can_frame
{
    u64 timestamp;
    u32 can_id;
    u8 pad;
    u8 res0;
    u8 len8_dlc;
    u8 data[CAN_MAX_DLEN];
};
```

> **timestamp** Timestamp in microseconds

> **can_id** 32 bit CAN_ID + EFF/RTR/ERR flags

> **pad** Padding byte

> **res0** Reserved / padding byte

> **len8_dlc** Optional DLC for 8 byte payload length (9 ..

> **data** CAN payload data (0 .. 8 bytes, up to 15 with len8_dlc)

<!-- tabs:end -->
