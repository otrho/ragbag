#include <gba.h>

#include "backgrounds.h"
#include "puzzle.h"
#include "sprites.h"
#include "util.h"
#include "prng.h"

// -------------------------------------------------------------------------------------------------

static void init_puzzle_gfx() {
  VBlankIntrWait();

  // Set the video mode to 0, enable bg 1, enable sprites, 1d mapping for sprites.
  SetMode(MODE_0 | BG1_ENABLE | OBJ_ENABLE | OBJ_1D_MAP);

  init_puzzle_bg();
  init_puzzle_sprites(224, 0);
}

// -------------------------------------------------------------------------------------------------

#define SCREEN_MAX_WIDTH 15
#define SCREEN_MAX_HEIGHT 10

struct CursorScroll {
  // These are mutable state, all in the pieces coordinates, not pixels.
  s32 cursor_x, cursor_y;
  s32 cursor_screen_x, cursor_screen_y;
  s32 puzzle_scroll_x, puzzle_scroll_y;

  // These are const.
  s32 puzzle_max_scroll_x, puzzle_max_scroll_y;
  s32 puzzle_width, puzzle_height;
  s32 origin_x, origin_y;
};

static void init_cursor_scroll(struct CursorScroll* cursor_scroll,
                               s32 width, s32 height, s32 origin_x, s32 origin_y) {
  cursor_scroll->cursor_x = 0; cursor_scroll->cursor_y = 0;
  cursor_scroll->cursor_screen_x = 0; cursor_scroll->cursor_screen_y = 0;
  cursor_scroll->puzzle_scroll_x = 0; cursor_scroll->puzzle_scroll_y = 0;
  cursor_scroll->puzzle_max_scroll_x = width - SCREEN_MAX_WIDTH;
  cursor_scroll->puzzle_max_scroll_y = height - SCREEN_MAX_HEIGHT;
  cursor_scroll->puzzle_width = width; cursor_scroll->puzzle_height = height;
  cursor_scroll->origin_x = origin_x; cursor_scroll->origin_y = origin_y;
}

static void centre_puzzle(struct CursorScroll* cursor_scroll) {
  // To put the origin in the centre, we want it at 15/2,10/2 == 7,5, but we need to keep it within
  // the bounds.
  s32 tmp_x = max(cursor_scroll->origin_x - 7, 0);
  cursor_scroll->puzzle_scroll_x = min(tmp_x, cursor_scroll->puzzle_max_scroll_x);
  s32 tmp_y = max(cursor_scroll->origin_y - 5, 0);
  cursor_scroll->puzzle_scroll_y = min(tmp_y, cursor_scroll->puzzle_max_scroll_y);

  // The cursor needs to be at the origin.
  cursor_scroll->cursor_x = cursor_scroll->origin_x;
  cursor_scroll->cursor_y = cursor_scroll->origin_y;
  cursor_scroll->cursor_screen_x = cursor_scroll->origin_x - cursor_scroll->puzzle_scroll_x;
  cursor_scroll->cursor_screen_y = cursor_scroll->origin_y - cursor_scroll->puzzle_scroll_y;
}

static void cursor_up(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_y > 0) {
    cursor_scroll->cursor_y--;
    if (cursor_scroll->cursor_screen_y > 0) {
      cursor_scroll->cursor_screen_y--;
    } else {
      cursor_scroll->puzzle_scroll_y--;
    }
  }
}

static void cursor_down(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_y < cursor_scroll->puzzle_height - 1) {
    cursor_scroll->cursor_y++;
    if (cursor_scroll->cursor_screen_y < SCREEN_MAX_HEIGHT - 1) {
      cursor_scroll->cursor_screen_y++;
    } else {
      cursor_scroll->puzzle_scroll_y++;
    }
  }
}

static void cursor_left(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_x > 0) {
    cursor_scroll->cursor_x--;
    if (cursor_scroll->cursor_screen_x > 0) {
      cursor_scroll->cursor_screen_x--;
    } else {
      cursor_scroll->puzzle_scroll_x--;
    }
  }
}

static void cursor_right(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_x < cursor_scroll->puzzle_width - 1) {
    cursor_scroll->cursor_x++;
    if (cursor_scroll->cursor_screen_x < SCREEN_MAX_WIDTH - 1) {
      cursor_scroll->cursor_screen_x++;
    } else {
      cursor_scroll->puzzle_scroll_x++;
    }
  }
}

static void update_screen_cursor_scroll(struct CursorScroll* cursor_scroll) {
      REG_BG1HOFS = (u16)cursor_scroll->puzzle_scroll_x * 16;   // Pixels...
      REG_BG1VOFS = (u16)cursor_scroll->puzzle_scroll_y * 16;
      move_cursor((u16)cursor_scroll->cursor_screen_x,
                  (u16)cursor_scroll->cursor_screen_y);
}

// -------------------------------------------------------------------------------------------------

static void redraw_puzzle_screen(struct CursorScroll* cursor_scroll) {
  for (s32 y = 0; y < SCREEN_MAX_HEIGHT; y++) {
    for (s32 x = 0; x < SCREEN_MAX_WIDTH; x++) {
      u16 palette =
        puzzle_piece_is_lit(cursor_scroll->puzzle_scroll_x + x,
                            cursor_scroll->puzzle_scroll_y + y) ? LIT_PALETTE : DULL_PALETTE;
      update_puzzle_board(cursor_scroll->puzzle_scroll_x + x,
                          cursor_scroll->puzzle_scroll_y + y,
                          palette);
    }
  }
}

// -------------------------------------------------------------------------------------------------
// Return whether the puzzle was solved.

static s32 puzzle_loop(struct CursorScroll* cursor_scroll) {
  centre_puzzle(cursor_scroll);

  VBlankIntrWait();
  enable_cursor();
  update_screen_cursor_scroll(cursor_scroll);
  render_lit_tiles(cursor_scroll->origin_x, cursor_scroll->origin_y);
  redraw_puzzle_screen(cursor_scroll);
  draw_puzzle_sprites();

  s32 total_pieces = cursor_scroll->puzzle_width * cursor_scroll->puzzle_height;

  s32 success = 0;
  for (s32 keep_going = 1; keep_going; ) {
    scanKeys();
    u16 keys_down = keysDown();

    if (keys_down & KEY_START) {
      // Abort.
      keep_going = 0;
    }

    /* if (keys_down & KEY_SELECT) {
      debug_cheat(cursor_scroll->puzzle_width, cursor_scroll->puzzle_height);
      render_lit_tiles(cursor_scroll->origin_x, cursor_scroll->origin_y);
    } */

    if (keys_down & KEY_UP) {
      cursor_up(cursor_scroll);
    }
    if (keys_down & KEY_DOWN) {
      cursor_down(cursor_scroll);
    }
    if (keys_down & KEY_LEFT) {
      cursor_left(cursor_scroll);
    }
    if (keys_down & KEY_RIGHT) {
      cursor_right(cursor_scroll);
    }

    if (keys_down & KEY_A) {
      rotate_piece(cursor_scroll->cursor_x, cursor_scroll->cursor_y, +1);
    }
    if (keys_down & KEY_B) {
      rotate_piece(cursor_scroll->cursor_x, cursor_scroll->cursor_y, -1);
    }

    // -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

    VBlankIntrWait();
    if (keys_down & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
      update_screen_cursor_scroll(cursor_scroll);
    }
    if (keys_down & (KEY_A | KEY_B)) {
      render_lit_tiles(cursor_scroll->origin_x, cursor_scroll->origin_y);
      if (total_solved(cursor_scroll->puzzle_width, cursor_scroll->puzzle_height) == total_pieces) {
        // Solved.
        success = 1;
        keep_going = 0;
      }
    }
    redraw_puzzle_screen(cursor_scroll);
    draw_puzzle_sprites();
  }

  disable_cursor();
  return success;
}

// -------------------------------------------------------------------------------------------------
// Return whether the puzzle was solved.

static s32 run_puzzle(s32 width, s32 height) {
  s32 origin_x;
  s32 origin_y;
  generate_puzzle(width, height, &origin_x, &origin_y);
  randomise_puzzle(width, height);

  VBlankIntrWait();
  struct CursorScroll cursor_scroll;
  init_cursor_scroll(&cursor_scroll, width, height, origin_x, origin_y);
  return puzzle_loop(&cursor_scroll);
}

// -------------------------------------------------------------------------------------------------

static void show_logo() {
}

// -------------------------------------------------------------------------------------------------

static s32 wait_for_key(s32 pause_frame_count, s32 timeout_frame_count) {
  s32 count = 0;

  while (1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_down = keysDown();

    if ((count > pause_frame_count && keys_down != 0) ||
        (timeout_frame_count != 0 && count > pause_frame_count + timeout_frame_count)) {
      return count;
    }

    count++;
  }
}

// -------------------------------------------------------------------------------------------------

static s32 show_title() {
  // Put the title at row 6.  It's 3 tiles high.
  generate_title(6);

  // Draw it.
  VBlankIntrWait();
  for (s32 y = 0; y < 10; y++) {
    for (s32 x = 0; x < 15; x++) {
      u16 pal_idx = (y >= 6 && y < 9) ? LIT_PALETTE : DULL_PALETTE;
      update_puzzle_board(x, y, pal_idx);
    }
  }
  draw_puzzle_sprites();

  // Wait for button press, no timeout.
  return wait_for_key(60, 0);
}

// -------------------------------------------------------------------------------------------------

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  show_logo();

  // The title needs the puzzle tiles.
  init_puzzle_gfx();

  // Small puzzle...
  s32 width = 15, height = 10;
  while (1) {
    s32 seed = show_title();
    prng_seed((u16)(seed | (seed << 8)));

    if (run_puzzle(width, height)) {
      palette_flash_screen(20);
      wait_for_key(30, 90);
    }
  }

  return 0;
}

// -------------------------------------------------------------------------------------------------
