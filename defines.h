#ifndef _DEFINES_H_
#define _DEFINES_H_

#define CAVE_MIN_HEIGHT 4
#define CAVE_MAX_HEIGHT 12

#define CAVE_MIN_WIDTH  4
#define CAVE_MAX_WIDTH  12

#define W 800
#define H 450
#define GPU_W 800
#define GPU_H 450
#define GLYPH_W 8
#define GLYPH_H 8
#define GLYPH_SCALE 1
#define TERM_W (W/GLYPH_W)
#define TERM_H (H/GLYPH_H)

#define MAP_COLS (TERM_W)
#define MAP_ROWS (TERM_H)

#define CARD_COUNT 5
#define CARD_W 8
#define CARD_H 8
#define CARD_DECK_MAX 5

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask 0xff000000
#define gmask 0x00ff0000
#define bmask 0x0000ff00
#define amask 0x000000ff
#else
#define rmask 0x000000ff
#define gmask 0x0000ff00
#define bmask 0x00ff0000
#define amask 0xff000000
#endif

#define map_param struct rune map[MAP_ROWS][MAP_COLS]
#define map_rune_char(r,c) (map[(r)][(c)].ch[map[(r)][(c)].i])
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#endif
