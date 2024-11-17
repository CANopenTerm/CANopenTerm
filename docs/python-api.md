# Python API

The Python API offers an alternative to the Lua scripting API.
It is based on pocketpy, a lightweight Python interpreter.
For a comprehensive overview of all features with respect to
cpython, please refer to the official documentation:
[PocketPy Features](https://pocketpy.dev/features/basic/).

## CAN Database File (DBC)

### dbc_decode()

<!-- tabs:start -->
<!-- tab:Description -->
```python
str dbc_decode (can_id, [data])
```

> **can_id** CAN-ID.

> **data** Data, default is `0`.

**Returns**: Decoded output as a `str`.

<!-- tab:Example -->
```python


```
<!-- tabs:end -->

### dbc_find_id_by_name()

<!-- tabs:start -->
<!-- tab:Description -->
```python
int dbc_find_id_by_name (search)
```

> **search** A case-insensitive sub`str` to search within message names.

**Returns**: CAN-ID or `None`.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### dbc_load()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool dbc_load (filename)
```

> **filename** DBC file name.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

## Network management (NMT)

!> The **command** parameter supports the following commands:

| Command | Description                    |
| ------- | ------------------------------ |
| 0x01    | Start (go to Operational)      |
| 0x02    | Stop (go to Stopped)           |
| 0x80    | Go to Pre-operational          |
| 0x81    | Reset node (Application reset) |
| 0x82    | Reset communication            |

### nmt_send_command()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool nmt_send_command (node_id, command, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **command** NMT command code.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output, default is `None`.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

## Process data objects (PDO)

It is possible to create up to 632 asynchronous PDOs, which are then
sent cyclically at the specified interval.

!>The following **CAN-IDs** can be used:

| From  | To    | Description |
| ----- | ----- | ----------- |
| 0x000 | 0x07f | Node-ID     |
| 0x181 | 0x1ff | TPDO1       |
| 0x281 | 0x2ff | TPDO2       |
| 0x381 | 0x3ff | TPDO3       |
| 0x481 | 0x4ff | TPDO4       |

### pdo_add()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool pdo_add (can_id, event_time_ms, length, [data], [show_output], [comment])
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### pdo_del()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool pdo_del (can_id, [show_output], [comment])
```

> **can_id** CAN-ID.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output, default is `None`.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

## Service data objects (SDO)

### sdo_lookup_abort_code()

<!-- tabs:start -->
<!-- tab:Description -->
```python
str sdo_lookup_abort_code (abort_code)
```

> **abort_code** SDO abort code.

**Returns**: Abort code description (`str`).

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### sdo_read()

<!-- tabs:start -->
<!-- tab:Description -->
Read SDO (expedited or segmented).

```python
tuple sdo_read (node_id, index, sub_index, [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.
              If ommited, the description from the CANopen object dictionary is used.

**Returns**:  

Expedited: `(int, None)`, or `(int, str)` if printable  
Segmented: `(str, str)`  
On failure: `(None, None)`

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### sdo_write()

<!-- tabs:start -->
<!-- tab:Description -->
Write expedited SDO.

```python
bool sdo_write (node_id, index, sub_index, length, [data], [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### sdo_write_file()

<!-- tabs:start -->
<!-- tab:Description -->
Write file (block transfer).

```python
bool sdo_write_file (node_id, index, sub_index, filename)
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **filename** The name of the file to be sent.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### sdo_write_string()

<!-- tabs:start -->
<!-- tab:Description -->
Write string (segmented transfer).

```python
bool sdo_write_string (node_id, index, sub_index, "[data]", [show_output], [comment])
```

> **node_id** CANopen Node-ID.

> **index** Index.

> **sub_index** Sub-Index.

> **data** The `str` to be written.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### dict_lookup()

<!-- tabs:start -->
<!-- tab:Description -->
Read CANopenTerm from built-in object directory.

```python
str dict_lookup (index, sub_index)
```

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

> **index** Index.

> **sub_index** Sub-Index.

**Returns**: a `str` or `None`.

## Generic CAN CC interface

### can_read()

<!-- tabs:start -->
<!-- tab:Description -->
```python
tuple can_read ()
```

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

**Returns**: (id, length, data, timestamp in μs), or `None` on failure.

### can_write()

<!-- tabs:start -->
<!-- tab:Description -->
```python
bool can_write (can_id, data_length, [data], [is_extended], [show_output], [comment])
```

> **can_id** CAN-ID.

> **length** Data length in bytes.

> **data** Data, default is `0`.

> **is_extended** Extended frame, default is `False`.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `True` on success, `False` on failure.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

## Miscellaneous

### delay_ms()

<!-- tabs:start -->
<!-- tab:Description -->
```python
None delay_ms ([delay_in_ms], [show_output], [comment])
```

> **delay_in_ms** Delay in milliseconds, default is 1000.

> **show_output** Show formatted output, default is `False`.

> **comment** Comment to show in formatted output.

**Returns**: `None`.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### key_is_hit()

<!-- tabs:start -->
<!-- tab:Description -->
```python
key_is_hit ()
```

**Returns**: `True` or `False`.

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

### print_heading()

<!-- tabs:start -->
<!-- tab:Description -->
```python
print_heading (heading)
```

<!-- tab:Example -->
```python
```
<!-- tabs:end -->

> Heading to be printed.

**Returns**: `None`.
