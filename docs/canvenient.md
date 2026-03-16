# CANvenient

!> CANvenient is an abstraction layer for multiple CAN APIs on
   Windows and Linux. It is currently under development and not
   yet released.

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
It needs to be called before any other function, as it initializes
the internal structures and detects the available CAN interfaces.

```c
int can_find_interfaces(void)
```

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
if (0 == can_find_interfaces())
{
    for (int i = 0; i < CAN_MAX_INTERFACES; i++)
    {
        char name[256] = {0};
        can_get_name(i, name, sizeof(name));

        if (0 == can_open(i))
        {
            printf("[%d] %s -> opened\n", i, name);
        }
    }
}

can_free_interfaces();
```
<!-- tabs:end -->

### can_free_interfaces()

<!-- tabs:start -->
<!-- tab:Description -->
Deinitializes and frees the memory allocated for the CAN interfaces detected by `can_find_interfaces()`.

!> It is safe to call this function, even if `can_find_interfaces()` failed or
   returned `0` interfaces, as it will handle the cleanup gracefully.

```c
void can_free_interfaces(void)
```

**Returns**: Nothing.

<!-- tabs:end -->

### can_open()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open(int index)
```

> **index** CAN interface index.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_open_fd()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open_fd(int index)
```

> **index** CAN interface index.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_close()

<!-- tabs:start -->
<!-- tab:Description -->
```c
void can_close(int index)
```

> **index** CAN interface index.

**Returns**: Nothing.

<!-- tabs:end -->


### can_get_name()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_get_name(int index, char* name_buf, size_t buf_size);
```
> **index** CAN interface index.

> **name_buf** Buffer to store the name of the CAN interface.

> **buf_size** Size of the name buffer.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_send()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_send(int index, struct can_frame* frame)
```

> **index** CAN interface index.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_recv()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_recv(int index, struct can_frame* frame)
```

> **index** CAN interface index.

**Returns**: `0` on success, `-1` on failure.

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
