#pragma once

#include <gba_types.h>

void generate_title();

void generate_puzzle(s32 width, s32 height, s32* origin_x, s32* origin_y);
void randomise_puzzle(s32 width, s32 height);

void rotate_piece(s32 x, s32 y, s32 dir);
void update_puzzle_board(s32 x, s32 y, u16 pal_idx);
s32 total_solved(s32 width, s32 height);

void render_lit_tiles(s32 x, s32 y);
s32 puzzle_piece_is_lit(s32 x, s32 y);

void debug_cheat(s32 width, s32 height);
