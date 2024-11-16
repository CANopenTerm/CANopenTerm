// Hello, world.

#include "misc.h"
#include "nmt.h"

print_heading("Hello, world.");
nmt_send_command(0x00, NMT_OPERATIONAL, true, NULL);

while (key_is_hit() == 0)
{
    delay_ms(10);
}
