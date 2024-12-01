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
    os_printf("<> codb2json\n");
    os_printf("Copyright (c) 2024, Michael Fitzmayer.\n\n");

    return codb2json(argc, argv);
}
