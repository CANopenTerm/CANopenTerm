/** @file server.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef SERVER_H
#define SERVER_H

#include "command.h"
#include "os.h"

#define SERVER_DEFAULT_HOST "localhost"
#define SERVER_DEFAULT_PORT 4120

status_t server_init(const char* host, uint16 port);
bool server_update(char command[COMMAND_BUFFER_SIZE]);
bool server_is_quit_requested(void);
void server_deinit(void);

#endif /* SERVER_H */
