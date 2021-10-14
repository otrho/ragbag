#pragma once

#include <gba_types.h>

void generate_puzzle(s32 width, s32 height, s32* origin_x, s32* origin_y);

void rotate_piece(s32 x, s32 y, s32 dir);
void update_puzzle_board(s32 x, s32 y);
