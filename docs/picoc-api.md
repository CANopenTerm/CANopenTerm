# PicoC API

!> Support for PicoC is currently being developed and is expected to become
   available in Version 1.0.5 at the earliest.

## Generic CAN interface

To use the CAN interface, include the following header file:

```C
#include "can.h"
```

### can_message_t

```C
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

### can_read()

<!-- tabs:start -->
<!-- tab:Description -->
```lua
int can_read (can_message_t* message)
```

> **message** A pointer of type [can_message_t](#can_message_t).

**Returns**: 0 on success, 1 on failure.

<!-- tab:Example -->
```c
#include "can.h"

can_message_t msg;

while (msg.id != 0x721) {
  can_read(&msg);
}

printf("ID: %d\n", msg.id);
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

### `#define`
Macros are implemented but have some limitations. They can only be used
as part of expressions and operate a bit like functions. Since they're used in
expressions they must result in a value.

### `#if/#ifdef/#else/#endif`
The conditional compilation operators are implemented, but have some limitations.
The operator "defined()" is not implemented. These operators can only be used at
statement boundaries.

### `#include`
Includes are supported however the level of support depends on the specific port
of PicoC on your platform. Linux/UNIX and Windows support #include fully.

### Function declarations
These styles of function declarations are supported:

```C
int my_function(char param1, int param2, char *param3)
{
   ...
}

int my_function(char param1, int param2, char *param3) {
   ...
}
```

The old "K&R" form of function declaration is not supported.

### Predefined macros
A few macros are pre-defined:

* PICOC_VERSION - gives the picoc version as a string eg. "v2.1 beta r524"

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
