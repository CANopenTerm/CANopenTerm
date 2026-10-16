/** @file client.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2022-2026, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef CLIENT_H
#define CLIENT_H

#include "os.h"

#define CLIENT_ESCAPE_KEY 0x1d /* Ctrl+] */

/* Connects to a running server instance and runs an interactive terminal session.
 * Blocks until the server closes the connection or the escape key is pressed. */
status_t client_connect(const char* host, uint16 port);

#endif /* CLIENT_H */
