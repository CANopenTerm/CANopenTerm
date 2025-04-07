

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius, bool fill);
/** @file primitives.h
 *
 *  A versatile software tool to analyse and configure CANopen devices.
 *
 *  Copyright (c) 2025, Michael Fitzmayer. All rights reserved.
 *  SPDX-License-Identifier: MIT
 *
 **/

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "os.h"

void draw_circle(os_renderer* renderer, int cx, int cy, int radius, bool fill);

#endif /* PRIMITIVES_H */
