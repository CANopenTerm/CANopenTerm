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
int can_find_interfaces(const struct can_iface* iface[], const int* count)
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

### can_free_interfaces();

<!-- tabs:start -->
<!-- tab:Description -->
Deinitializes and frees the memory allocated for the CAN interfaces detected by `can_find_interfaces()`.

!> It is safe to call this function, even if `can_find_interfaces()` failed or
   returned `0` interfaces, as it will handle the cleanup gracefully.

```c
void can_free_interfaces(const struct can_iface* iface[], const int count);
```

> **iface** A pointer to an array of `can_iface` structures that will be populated with the details of the detected CAN interfaces.

> **count** A pointer to an integer that will be set to the number of CAN interfaces detected and stored in the `iface` array.

**Returns**: `0` on success, `-1` on failure.

<!-- tab:Example -->
```c
struct can_iface* iface[] = {0};
int count = 0;

can_find_interfaces(iface, &count))
can_free_interfaces(iface, count);
```
<!-- tabs:end -->

### can_open();

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open(struct can_iface* iface);
```

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_open_fd();

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_open_fd(struct can_iface* iface);
```

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_close();

<!-- tabs:start -->
<!-- tab:Description -->
```c
void can_close(const struct can_iface* iface);
```

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_send();

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_send(struct can_iface* iface, struct can_frame* frame);
```

<!-- tab:Example -->
```c
```
<!-- tabs:end -->

### can_recv();

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_recv(struct can_iface* iface, struct can_frame* frame);
```

<!-- tab:Example -->
```c
```
<!-- tabs:end -->
