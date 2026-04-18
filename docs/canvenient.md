# CANvenient

CANvenient is an abstraction layer for multiple CAN APIs on Windows
and Linux. It provides a unified interface for CAN communication,
allowing developers to write code that is portable across different
platforms and CAN hardware.

It can be found on GitHub: 
[CANopenTerm/CANvenient](https://github.com/CANopenTerm/CANvenient).

## Supported Back-Ends

The following back-ends are currently implemented:	

- Ixxat VCI
- Kvaser CANlib
- PCAN-Basic
- SocketCAN
- Softing CAN Layer 2
- MHS Elektronik Tiny-CAN

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

can_release_interfaces();
```
<!-- tabs:end -->

### can_release_interfaces()

<!-- tabs:start -->
<!-- tab:Description -->
Deinitializes and frees the memory allocated for the CAN interfaces detected by `can_find_interfaces()`.

!> It is safe to call this function, even if `can_find_interfaces()` failed or
   returned `0` interfaces, as it will handle the cleanup gracefully.

```c
void can_release_interfaces(void)
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

### can_close()

<!-- tabs:start -->
<!-- tab:Description -->
```c
void can_close(int index)
```

> **index** CAN interface index.

**Returns**: Nothing.

<!-- tabs:end -->

### can_release()

<!-- tabs:start -->
<!-- tab:Description -->
Releases the CAN interface at the specified index, freeing any resources
associated with it. Implies `can_close()` if the interface is still open.
After calling this function, the index may no longer be valid.

```c
void can_release(int index)
```

> **index** CAN interface index.

**Returns**: Nothing.

<!-- tabs:end -->

### can_update()

<!-- tabs:start -->
<!-- tab:Description -->
This function updates the internal state of the CAN interface at the specified index.
If it returns `-1`, it indicates that the CAN interface is no longer available
(e.g., it was unplugged).

```c
void can_update(int index)
```

> **index** CAN interface index.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_get_baudrate()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_get_baudrate(int index, enum can_baudrate* baud)
```

> **index** CAN interface index.

> **baud** Pointer to a variable to store the baud rate of the CAN interface.
  See [can_baudrate](#can_baudrate) enumeration for
  available baud rates.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_get_error()

<!-- tabs:start -->
<!-- tab:Description -->
Retrieves the last error reason for the specified CAN interface index.
This function can be used to get more information about why a previous
operation (like `can_open()`, `can_send()`, etc.) failed.

```c
void can_get_error(char* reason_buf, size_t buf_size)
```

> **reason_buf** Buffer to store the error reason string.

> **buf_size** Size of the name buffer.

**Returns**: Nothing.

<!-- tabs:end -->

### can_get_name()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_get_name(int index, char* name_buf, size_t buf_size)
```

> **index** CAN interface index.

> **name_buf** Buffer to store the name of the CAN interface.

> **buf_size** Size of the name buffer.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_set_baudrate()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_set_baudrate(int index, enum can_baudrate baud)
```
> **index** CAN interface index.

> **baud** Baud rate to set for the CAN interface.
  See [can_baudrate](#can_baudrate) enumeration for
  available baud rates.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_send()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_send(int index, struct can_frame* frame)
```

> **index** CAN interface index.

> **frame** Pointer to a `can_frame` structure containing the CAN frame to be sent.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

### can_recv()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_recv(int index, struct can_frame* frame, u64* timestamp)
```

> **index** CAN interface index.

> **frame** Pointer to a `can_frame` structure to store the received CAN frame.

> **timestamp** Pointer to a variable to store the timestamp of the received CAN frame in microseconds.

**Returns**: `0` on success, `-1` on failure.

<!-- tabs:end -->

## Enumerations

### can_baudrate

| Value         | Description   |
| :------------ | :------------ |
| CAN_BAUD_1M   | 1 MBit/s      |
| CAN_BAUD_800K | 800 kBit/s    |
| CAN_BAUD_500K | 500 kBit/s    |
| CAN_BAUD_250K | 250 kBit/s    |
| CAN_BAUD_125K | 125 kBit/s    |
| CAN_BAUD_100K | 100 kBit/s    |
| CAN_BAUD_95K  | 95.238 kBit/s |
| CAN_BAUD_83K  | 83.333 kBit/s |
| CAN_BAUD_50K  | 50 kBit/s     |
| CAN_BAUD_47K  | 47.619 kBit/s |
| CAN_BAUD_33K  | 33.333 kBit/s |
| CAN_BAUD_20K  | 20 kBit/s     |
| CAN_BAUD_10K  | 10 kBit/s     |
| CAN_BAUD_5K   | 5 kBit/s      | 

## Structures

### can_frame

<!-- tabs:start -->
<!-- tab:Description -->

This is a drop-in replacement for the Linux `can_frame` struct defined in `linux/can.h`.

```c
struct can_frame {
	canid_t can_id;

	union {
		u8 len;
		u8 can_dlc;

	};

	u8 __pad;
	u8 __res0;
	u8 len8_dlc;
	u8 data[CAN_MAX_DLEN];
};
```

> **can_id** 32 bit CAN_ID + EFF/RTR/ERR flags.

> **len/can_dlc** frame payload length in byte (0 .. CAN_MAX_DLEN).

> **__pad** Padding byte.

> **__res0** Reserved / padding byte.

> **len8_dlc** Optional DLC for 8 byte payload length (9 ..

> **data** CAN payload data (0 .. 8 bytes, up to 15 with len8_dlc).

<!-- tabs:end -->
