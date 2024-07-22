# PicoC API

!> Support for PicoC is currently being developed and is expected to
   become available in Version 1.0.5 at the earliest.

## Capabilities and Limitations

!> The PicoC API offers an alternative to Lua for scripting and automation,
   but with some limitations. Notably, PicoC lacks support for default
   parameters and multiple return values, which may limit script flexibility.
   Users should consider these constraints to determine if PicoC meets their
   project needs.

!> Scripts are slightly simpler than standard C programs because, **A**)
   all the system headers are included automatically for you so you don't
   need to include them in your file/s and **B**) scripts don't require a
   `main()` function; they have statements that are run directly from the
   top of a file to the bottom.

## CAN Database File (DBC)

To use the DBC file parser interface, include the following header file:

```c
#include "dbc.h"
```

### dbc_decode()

<!-- tabs:start -->
<!-- tab:Description -->
```c
char* dbc_decode (int can_id, char* data)
```
> **can_id** CAN-ID.

> **data** 8-byte data buffer.

**Returns**: A string containing the decoded message.

<!-- tab:Example -->
```c
#include "dbc.h"

char data[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

printf("%s\n", dbc_decode(0x123, data));
```
<!-- tabs:end -->

### dbc_find_id_by_name()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int dbc_find_id_by_name (int* can_id, char* search)
```

> **can_id** CAN-ID.

> **search** A case-insensitive substring to search within message names.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "dbc.h"

int id = 0

dbc_load("Edison_Model_3.dbc");

if (dbc_find_id_by_name(&id, "engine_speed")) {
   printf("%Xh\n, id);
}
```
<!-- tabs:end -->

### dbc_load()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int dbc_load (char* filename)
```

> **filename** DBC file name.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "dbc.h"

if (! dbc_load("Edison_Model_3.dbc")) {
   printf("Failed to load DBC file\n");
}
```
<!-- tabs:end -->

## Network management (NMT)

To use the NMT interface, include the following header file:

```c
#include "nmt.h"
```

### nmt_command_t

```c
typedef enum
{
  NMT_OPERATIONAL     = 0x01,
  NMT_STOP            = 0x02,
  NMT_PRE_OPERATIONAL = 0x80,
  NMT_RESET_NODE      = 0x81,
  NMT_RESET_COMM      = 0x82

} nmt_command_t;
```

### nmt_send_command()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int nmt_send_command (int node_id, nmt_command_t command, int show_output, char* comment)
```

> **node_id** CANopen Node-ID.

> **command** NMT command code of type [nmt_command_t](#nmt_command_t).

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "nmt.h"

nmt_send_command(0x01, NMT_OPERATIONAL, 1, "Switch to operational state");
```
<!-- tabs:end -->

## Process data objects (PDO)

It is possible to create up to 632 asynchronous PDOs, which are then
sent cyclically at the specified interval.

To use the PDO interface, include the following header file:

```c
#include "pdo.h"
```

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
```c
int pdo_add (int can_id, unsigned int event_time_ms, int length, char* data, int show_output, char* comment);
```

> **can_id** CAN-ID.

> **event_time_ms** Event time in milliseconds.

> **length** Data length (0 to 8).

> **data** Data buffer.

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "misc.h"
#include "pdo.h"

char data[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

pdo_add(0x181, 100, 8, data, 1, "New TPDO!");
delay_ms(1000);
pdo_del(0x181, 1, NULL);
```
<!-- tabs:end -->

### pdo_del()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int pdo_del (int can_id, int show_output, char* comment)
```

> **can_id** CAN-ID.

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "misc.h"
#include "pdo.h"

char data[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

pdo_add(0x181, 100, 8, data, 1, "New TPDO!");
delay_ms(1000);
pdo_del(0x181, 1, NULL);
```
<!-- tabs:end -->

## Service data objects (SDO)

To use the SDO interface, include the following header file:

```c
#include "sdo.h"
```

## sdo_abort_code_t

```c
typedef enum
{
    ABORT_TOGGLE_BIT_NOT_ALTERED             = 0x05030000,
    ABORT_SDO_PROTOCOL_TIMED_OUT             = 0x05040000,
    ABORT_CMD_SPECIFIER_INVALID_UNKNOWN      = 0x05040001,
    ABORT_INVALID_BLOCK_SIZE                 = 0x05040002,
    ABORT_INVALID_SEQUENCE_NUMBER            = 0x05040003,
    ABORT_CRC_ERROR                          = 0x05040004,
    ABORT_OUT_OF_MEMORY                      = 0x05040005,
    ABORT_UNSUPPORTED_ACCESS                 = 0x06010000,
    ABORT_ATTEMPT_TO_READ_WRITE_ONLY         = 0x06010001,
    ABORT_ATTEMPT_TO_WRITE_READ_ONLY         = 0x06010002,
    ABORT_OBJECT_DOES_NOT_EXIST              = 0x06020000,
    ABORT_OBJECT_CANNOT_BE_MAPPED            = 0x06040041,
    ABORT_WOULD_EXCEED_PDO_LENGTH            = 0x06040042,
    ABORT_GENERAL_INCOMPATIBILITY_REASON     = 0x06040043,
    ABORT_GENERAL_INTERNAL_INCOMPATIBILITY   = 0x06040047,
    ABORT_ACCESS_FAILED_DUE_HARDWARE_ERROR   = 0x06060000,
    ABORT_DATA_TYPE_DOES_NOT_MATCH           = 0x06070010,
    ABORT_DATA_TYPE_LENGTH_TOO_HIGH          = 0x06070012,
    ABORT_DATA_TYPE_LENGTH_TOO_LOW           = 0x06070013,
    ABORT_SUB_INDEX_DOES_NOT_EXIST           = 0x06090011,
    ABORT_INVALID_VALUE_FOR_PARAMETER        = 0x06090030,
    ABORT_VALUE_FOR_PARAMETER_TOO_HIGH       = 0x06090031,
    ABORT_VALUE_FOR_PARAMETER_TOO_LOW        = 0x06090032,
    ABORT_MAX_VALUE_LESS_THAN_MIN_VALUE      = 0x06090036,
    ABORT_RESOURCE_NOT_AVAILABLE             = 0x060a0023,
    ABORT_GENERAL_ERROR                      = 0x08000000,
    ABORT_DATA_CANNOT_BE_TRANSFERRED         = 0x08000020,
    ABORT_DATA_CANNOT_TRANSFERRED_LOCAL_CTRL = 0x08000021,
    ABORT_DATA_CANNOT_TRANSFERRED_DEV_STATE  = 0x08000022,
    ABORT_NO_OBJECT_DICTIONARY_PRESENT       = 0x08000023,
    ABORT_NO_DATA_AVAILABLE                  = 0x08000024

} sdo_abort_code_t;
```

## sdo_lookup_abort_code()

<!-- tabs:start -->
<!-- tab:Description -->
```c
char* sdo_lookup_abort_code (sdo_abort_code_t abort_code)
```

> **abort_code** SDO abort code of type [sdo_abort_code_t](#sdo_abort_code_t).

> **Returns**: A string containing the abort code description.

<!-- tab:Example -->
```c
#include "sdo.h"

printf("%s\n", sdo_lookup_abort_code(ABORT_CRC_ERROR));
printf("%s\n", sdo_lookup_abort_code(0x06060000)); // Hardware error.
```
<!-- tabs:end -->

## sdo_read()

<!-- tabs:start -->
<!-- tab:Description -->
```c
char* sdo_read (unsigned int* result, int node_id, int index, int sub_index, int show_output, char* comment)
```

> **result** A pointer to store the result.

> **node_id** CANopen Node-ID.

> **index** Object index.

> **sub_index** Object sub-index.

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: A string (if printable) or NULL.

<!-- tab:Example -->
```c
#include "sdo.h"

unsigned int result = 0;
char*        result_str = sdo_read(&result, 0x123, 0x1008, 0x00, 1, NULL);

printf("%Xh %s\n", result, result_str);
```
<!-- tabs:end -->

## sdo_write()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int sdo_write (int node_id, int index, int sub_index, int length, char* data, int show_output, char* comment);
```

> **node_id** CANopen Node-ID.

> **index** Object index.

> **sub_index** Object sub-index.

> **length** Data length (0 to 8).

> **data** Data buffer.

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "sdo.h"

char data[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

sdo_write(0x123, 0x2000, 0x01, 0x08, data, 1, "Hello, world.");
```
<!-- tabs:end -->

## sdo_write_file()

<!-- tabs:start -->
<!-- tab:Description -->
```c
```

<!-- tab:Example -->
```c
#include "sdo.h"
```
<!-- tabs:end -->

## sdo_write_string()

<!-- tabs:start -->
<!-- tab:Description -->
```c
```

<!-- tab:Example -->
```c
#include "sdo.h"
```
<!-- tabs:end -->

## dict_lookup()

<!-- tabs:start -->
<!-- tab:Description -->
```c
char* dict_lookup (int index, int sub_index)
```

> **index** Object index.

> **sub_index** Object sub-index.

**Returns**: A string containing the object description or NULL.

<!-- tab:Example -->
```c
#include "sdo.h"

/* Manufacturer device name. */
printf("%s\n", dict_lookup, 0x1008, 0x00);
```
<!-- tabs:end -->

## Generic CAN interface

To use the CAN interface, include the following header file:

```c
#include "can.h"
```

### can_message_t

```c
typedef struct can_message
{
  int  id;
  int  length;
  char data[0xff];
  long timestamp_us;
  int  is_extended;

} can_message_t;
```

> **id** CAN-ID.

> **length** Data length (0 to 8).

> **data** Data buffer.

> **timestamp_us** Timestamp in microseconds.

> **is_extended** Extended frame format.

!> The can_message_t struct in the PicoC API includes a data buffer with a size
   of 0xff. However, only the first 8 bytes of the buffer are relevant to the user.
   The remaining buffer space is used internally.

### can_read()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_read (can_message_t* message)
```

> **message** A pointer of type [can_message_t](#can_message_t).

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "can.h"

can_message_t msg;

while (msg.id != 0x123) {
  can_read(&msg);
}

printf("ID: %d\n", msg.id);
```
<!-- tabs:end -->

### can_write()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int can_write (can_message_t* message, int show_output, char* comment)
```

> **message** A pointer of type [can_message_t](#can_message_t).

> **show_output** Show output (boolean operation).

> **comment** Comment string or NULL.

**Returns**: 1 on success, 0 on failure.

<!-- tab:Example -->
```c
#include "can.h"

can_message_t msg;

msg.id      = 0x123;
msg.length  = 1;
msg.data[0] = 0x55;

can_write(&msg);
```
<!-- tabs:end -->

## Miscellaneous

To use the miscellaneous function, include the following header file:

```c
#include "misc.h"
```

### delay_ms()

<!-- tabs:start -->
<!-- tab:Description -->
```c
void delay_ms (unsigned int delay_in_ms)
```

> **delay_in_ms** Delay in milliseconds.

**Returns**: Nothing.

<!-- tab:Example -->
```c
while (key_is_hit() == 0) {
  delay_ms(10);
}
```
<!-- tabs:end -->

### key_is_hit()

<!-- tabs:start -->
<!-- tab:Description -->
```c
int key_is_hit (void)
```

**Returns**: 1 if a key is hit, 0 otherwise.

<!-- tab:Example -->
```c
while (key_is_hit() == 0) {
  delay_ms(10);
}
```
<!-- tabs:end -->

### print_heading()

<!-- tabs:start -->
<!-- tab:Description -->
```c
void print_heading (char* heading)
```

> **heading** The heading to be printed or NULL.

**Returns**: Nothing.

<!-- tab:Example -->
```c
#include "misc.h"
#include "nmt.h"

print_heading("A clever heading");
nmt_send_command(0x00, NMT_OPERATIONAL, 1, NULL);
```
<!-- tabs:end -->

## How PicoC differs from C90

PicoC is a tiny C language, not a complete implementation of C90. It doesn't
aim to implement every single feature of C90 but it does aim to be close enough
that most programs will run without modification.

PicoC also has scripting abilities which enhance it beyond what C90 offers.

### C preprocessor
There is no true preprocessor in PicoC. The most popular preprocessor features
are implemented in a slightly limited way.

### #define
Macros are implemented but have some limitations. They can only be used
as part of expressions and operate a bit like functions. Since they're used in
expressions they must result in a value.

### #if/#ifdef/#else/#endif
The conditional compilation operators are implemented, but have some limitations.
The operator `defined()` is not implemented. These operators can only be used at
statement boundaries.

### #include
Includes are supported however the level of support depends on the specific port
of PicoC on your platform. Linux/UNIX and Windows support #include fully.

### Function declarations/definitions
These styles of function declarations/definitions are supported:

```c
int my_function(char param1, int param2, char *param3)
{
   …
}
```

```c
int my_function(char param1, int param2, char *param3) {
   …
}
```

The old "K&R" form of function declaration/definition is **not** supported:

```c
int my_function(param1, param2, param3)
  char  param1;
  int   param2;
  char *param3;
{
   …
}
```

### Predefined macros
A few macros are pre-defined:

* `PICOC_VERSION` - gives the picoc version as a string eg. "v2.1 beta r524"

### Function pointers
Pointers to functions are currently not supported.

### Storage classes
Many of the storage classes in C90 really only have meaning in a compiler so
they're not implemented in picoc. This includes: static, extern, volatile,
register and auto. They're recognised but currently ignored.

### struct and unions
Structs and unions can only be defined globally. It's not possible to define
them within the scope of a function.

Bitfields in structs are not supported.

### goto
The goto statement is implemented but only supports forward gotos, not backward.
The rationale for this is that backward gotos are not necessary for any
"legitimate" use of goto.

Some discussion on this topic:

* http://www.cprogramming.com/tutorial/goto.html
* http://kerneltrap.org/node/553/2131
