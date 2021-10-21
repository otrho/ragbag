#include <stdlib.h>

#include <gba.h>

#include "backgrounds.h"
#include "puzzle.h"
#include "sprites.h"
#include "util.h"

// -------------------------------------------------------------------------------------------------

static void init_gfx() {
  VBlankIntrWait();

  // Set the video mode to 0, enable bg 1, enable sprites, 1d mapping for sprites.
  SetMode(MODE_0 | BG1_ENABLE | OBJ_ENABLE | OBJ_1D_MAP);

  init_puzzle_bg();
  init_sprites(224, 0);
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

static void init_tacho_timers() {
  // Timer0 counts from 731 to 1097 every frame, cascades into timer1.
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = 731;
  REG_TM1CNT_L = 0;
  REG_TM1CNT_H = 0;
}

static void restart_tacho() {
  // Enable both timers, timer0 @ freq/256, timer1 cascading.
  REG_TM0CNT_L = 0x82;
  REG_TM1CNT_L = 0x84;
}

static s32 g_tacho_max = 0;

static s32 read_tacho() {
  // Timer0 - 731 is 0..384 which is the full range of our tacho.
  s32 tacho_angle = REG_TM1CNT_H > 0 ? 384 : -(s32)(REG_TM0CNT_H - 713);

  // Reset timers.
  REG_TM0CNT_L = 0;
  REG_TM1CNT_L = 0;

  g_tacho_max = max(g_tacho_max, tacho_angle);
  return g_tacho_max;
}

// -------------------------------------------------------------------------------------------------

static void puzzle_loop(struct CursorScroll* cursor_scroll) {
  init_tacho_timers();

  centre_puzzle(cursor_scroll);

  VBlankIntrWait();
  enable_cursor();
  update_screen_cursor_scroll(cursor_scroll);
  render_lit_tiles(cursor_scroll->origin_x, cursor_scroll->origin_y);
  redraw_puzzle_screen(cursor_scroll);
  draw_sprites();

  while (1) {
    restart_tacho();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_START) {
      break;
    }

    if (keys_up & KEY_UP) {
      cursor_up(cursor_scroll);
    }
    if (keys_up & KEY_DOWN) {
      cursor_down(cursor_scroll);
    }
    if (keys_up & KEY_LEFT) {
      cursor_left(cursor_scroll);
    }
    if (keys_up & KEY_RIGHT) {
      cursor_right(cursor_scroll);
    }

    if (keys_up & KEY_A) {
      rotate_piece(cursor_scroll->cursor_x, cursor_scroll->cursor_y, +1);
    }
    if (keys_up & KEY_B) {
      rotate_piece(cursor_scroll->cursor_x, cursor_scroll->cursor_y, -1);
    }

    rotate_tacho(read_tacho());

    // -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

    VBlankIntrWait();
    if (keys_up & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
      update_screen_cursor_scroll(cursor_scroll);
    }
    if (keys_up & (KEY_A | KEY_B)) {
      render_lit_tiles(cursor_scroll->origin_x, cursor_scroll->origin_y);
    }
    redraw_puzzle_screen(cursor_scroll);
    draw_sprites();
  }
}

// -------------------------------------------------------------------------------------------------

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  // Using timer 2 to seed the PRNG.  The count between start and when the user first presses 'A'.
  REG_TM2CNT_H |= TIMER_START;

  // Load our backrounds and sprites into VRAM.
  init_gfx();

  while (1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_A) {
      break;
    }
  }

  srandom(REG_TM2CNT);

  // Small = 1 screen = 15x10
  // Medium = middle = 24x24
  // Large = entire bg = 32x32
  s32 width = 32;
  s32 height = 32;
  s32 origin_x;
  s32 origin_y;
  generate_puzzle(width, height, &origin_x, &origin_y);
  randomise_puzzle(width, height);

  VBlankIntrWait();
  struct CursorScroll cursor_scroll;
  init_cursor_scroll(&cursor_scroll, width, height, origin_x, origin_y);
  puzzle_loop(&cursor_scroll);

  Stop();
  return 0;
}

// -------------------------------------------------------------------------------------------------
