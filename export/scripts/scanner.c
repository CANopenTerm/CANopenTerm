/* CANopen node scanner
 *
 * Author:  Michael Fitzmayer
 * License: Public domain
 *
 */

#include "can.h"
#include "misc.h"
#include "nmt.h"
#include "sdo.h"

#define TIMEOUT_MS 3000
#define MAX_NODES  3

unsigned char nodes[MAX_NODES];
int           node_count = 0;
clock_t       start_time = clock();
double        elapsed_time_ms;
int           i;

nmt_send_command(0x00, 0x81, false, NULL); // Reset all nodes.

while (1)
{
    can_message_t* msg;

    elapsed_time_ms = (double)(clock() - start_time) / CLOCKS_PER_SEC * 1000.0;

    if (elapsed_time_ms >= TIMEOUT_MS)
    {
        break;
    }

    msg = can_read();
    if (msg != NULL)
    {
        // Wait for boot-up messages.
        if (msg->length == 1 && msg->data[0] == 0x00)
        {
            nodes[node_count++] = msg->id - 0x700;
            if (node_count >= MAX_NODES)
            {
                break;
            }
        }
    }

    if (key_is_hit())
    {
        break;
    }
}

for (i = 0; i < node_count; i++)
{
    unsigned char node_id  = nodes[i];
    char*         dev_name = NULL;
    unsigned int  result   = 0;

    dev_name = sdo_read(&result, node_id, 0x1008, 0x00, false, NULL);
    if (dev_name == NULL)
    {
        dev_name = "Unknown device";
    }

    print_heading(dev_name);
    sdo_read(&result, node_id, 0x1000, 0x00, true, NULL);
    sdo_read(&result, node_id, 0x1009, 0x00, true, NULL);
    sdo_read(&result, node_id, 0x100A, 0x00, true, NULL);
}

printf("\n");
