/** @file main.c
 *
 *  CANopen Profile data base to JSON converter.
 *
 *  Copyright (c) 2024, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#include <stdlib.h>
#include "codb2json.h"
#include "os.h"

int main(int argc, char* argv[])
{
    size_t i;
    bool_t format_output = IS_FALSE;

    os_printf("<> codb2json\n");
    os_printf("Copyright (c) 2024, Michael Fitzmayer.\n\n");

    if (argc < 2)
    {
        os_printf("Usage: %s <input file> [OPTION]\n", argv[0]);
        os_printf("    -f EDS            Format JSON output\n");
        return EXIT_SUCCESS;
    }
    else if (argc > 2)
    {
        for (i = 2; i < argc; i++)
        {
            if (0 == os_strcmp(argv[i], "-f"))
            {
                format_output = IS_TRUE;
            }
        }
    }

    return codb2json(argc, argv, format_output);
}
