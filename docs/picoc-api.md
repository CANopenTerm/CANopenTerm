# PicoC API

!> Support for PicoC is currently being developed and is expected to become
   available in Version 1.0.5 at the earliest.

!> When writing a PicoC script, you do not need to define a main function.
   PicoC scripts are executed from top to bottom without the need for an
   explicit entry point like a main function in traditional C programs.

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

To use the PDO interface, include the following header file:

```c
#include "pdo.h"
```

!> Not yet available.

## Service data objects (SDO)

To use the SDO interface, include the following header file:

```c
#include "sdo.h"
```

!> Not yet available.

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
int key_is_hit (void)

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
nmt_send_command(0x00, NMT_OPERATIONAL, true, NULL);
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
