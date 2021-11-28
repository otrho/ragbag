#pragma once

#include "gba_types.h"

void init_puzzle_sprites(u16 tacho_x, u16 tacho_y);
void draw_puzzle_sprites();

void enable_cursor();
void move_cursor(u16 x, u16 y);

void rotate_tacho(s16 angle);
