# CANvenient

!> CANvenient is an abstraction layer for multiple CAN APIs on Windows.
It is currently under development and not yet released.

It can be found in its own repository at
[CANvenient](https://github.com/CANopenTerm/CANvenient).

## API Reference

### can_find_interfaces()

<!-- tabs:start -->
<!-- tab:Description -->
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
        if (iface[i] != NULL)
        {
            printf("[%d] %s ->", i, iface[i]->name);
        }
    }
}

can_free_interfaces(iface, count);
```
<!-- tabs:end -->
