#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#include "defines.h"

#include "font8x8_basic.h"
#include "vga256.h"
#include "entity.h"
#include "rune.h"
#include "util.h"

#include "generate.h"

struct input {
    int up;
    int down;
    int left;
    int right;
} input;

struct deck {
    enum rune_type_enum card[CARD_DECK_MAX];
    int count; 
} deck;

struct entity_list entity_list;
SDL_Texture *font_texture;
SDL_Texture *gpu_texture;
SDL_Texture *card_texture;
int redraw = 0;
int level = 1;
double view_scale = 4.0;

static void die(int status)
{
    exit(status);
}

void render8x8(SDL_Surface *surface, int surface_x, int surface_y, unsigned char *bitmap)
{
    int x,y;
    int set;
    int color;
    Uint32 *p = (Uint32*)surface->pixels + (surface_y * (8*16) + surface_x);

    for (x=0; x < 8; x++) {
        for (y=0; y < 8; y++) {
            set = bitmap[x] & 1 << y;
            color = set ? 0xffffffff : 0xff00ffff;
            p[x * (8*16) + y] = color;
        }
    }
}

SDL_Surface *make_font()
{
    SDL_Surface *result = SDL_CreateRGBSurfaceWithFormat(0, 16 * 8, 16 * 8, 32, SDL_PIXELFORMAT_RGBA8888);
    for (int c = 0; c < 127; c++) {
        render8x8(result, (c%16)*8, (c/16)*8, font8x8_basic[c]);
    }
    return result;
}

static int deck_add_card(enum rune_type_enum rune_type)
{
    if (deck.count < CARD_DECK_MAX) {
        deck.card[deck.count++] = rune_type;
        return 1;
    }
    return 0;
}

static void render_card_in_deck(SDL_Renderer *renderer, int x, int y, int card_index, double angle, int scale)
{
    int dstx = x;
    int dsty = y;
    int srcx = (int)(card_index) * CARD_W;
    int srcy = 0;

    SDL_Rect src = { srcx, srcy, CARD_W, CARD_H};
    SDL_Rect dst = { dstx, dsty, GLYPH_W * scale, GLYPH_H * scale};
    SDL_SetTextureColorMod(card_texture, 0xff, 0xff, 0xff);

    SDL_RenderCopy(renderer, card_texture, &src, &dst);
}

static void render_card(SDL_Renderer *renderer, int x, int y, int card_index, double angle)
{
    int dstx = x * GLYPH_W * GLYPH_SCALE;
    int dsty = y * GLYPH_H * GLYPH_SCALE;
    int srcx = (int)(card_index) * CARD_W;
    int srcy = 0;

    SDL_Rect src = { srcx, srcy, CARD_W, CARD_H};
    SDL_Rect dst = { dstx, dsty, GLYPH_W*GLYPH_SCALE, GLYPH_H*GLYPH_SCALE };
    SDL_SetTextureColorMod(card_texture, 0xff, 0xff, 0xff);

    SDL_Point center = { .x = (CARD_W/2), .y = (CARD_H/2) };
    SDL_RenderCopyEx(renderer, card_texture, &src, &dst, angle, &center, SDL_FLIP_NONE);
}

static void render_rune_ex(SDL_Renderer *renderer, int x, int y, unsigned char c, double angle, int bg, int fg)
{
    int dstx = x * GLYPH_W * GLYPH_SCALE;
    int dsty = y * GLYPH_H * GLYPH_SCALE;
    int srcx = (int)(c % 16) * GLYPH_W;
    int srcy = (int)(c / 16) * GLYPH_H;

    int fg_r = vga256[fg*3+0] * 4;
    int fg_g = vga256[fg*3+1] * 4;
    int fg_b = vga256[fg*3+2] * 4;

    int bg_r = bg == -1 ? 0 : vga256[bg*3+0] * 4;
    int bg_g = bg == -1 ? 0 : vga256[bg*3+1] * 4;
    int bg_b = bg == -1 ? 0 : vga256[bg*3+2] * 4;

    SDL_Rect src = { srcx, srcy, GLYPH_W, GLYPH_H };
    SDL_Rect dst = { dstx, dsty, GLYPH_W*GLYPH_SCALE, GLYPH_H*GLYPH_SCALE };
    if (bg != -1) {
        SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 0xff);
        SDL_RenderFillRect(renderer, &dst);
    }
    if (fg != -1) {
        SDL_SetTextureColorMod(font_texture, fg_r, fg_g, fg_b);
    } else {
        SDL_SetTextureColorMod(font_texture, 0xff, 0xff, 0xff);
    }

    SDL_Point center = { .x = (GLYPH_W/2), .y = (GLYPH_H/2) };
    SDL_RenderCopyEx(renderer, font_texture, &src, &dst, angle, &center, SDL_FLIP_NONE);
}

static void render_entity(SDL_Renderer *renderer, struct entity *e, double angle)
{
    render_rune_ex(renderer, e->c, e->r, e->rune.ch[e->rune.i], angle, e->rune.bg, e->rune.fg);
}

static void render_rune(SDL_Renderer *renderer, struct rune *rune, int r, int c, double angle)
{
    switch (rune->type) {
        case rune_type_card_fire:
            render_card(renderer, c, r, 0, 0);
            break;
        case rune_type_card_ice:
            render_card(renderer, c, r, 1, 0);
            break;
        case rune_type_card_medical:
            render_card(renderer, c, r, 2, 0);
            break;
        case rune_type_card_slime:
            render_card(renderer, c, r, 3, 0);
            break;
        case rune_type_card_poison:
            render_card(renderer, c, r, 4, 0);
            break;
        default:
            render_rune_ex(renderer, c, r, rune->ch[rune->i], angle, rune->bg, rune->fg);
            break;
    }
}

static void render_deck(SDL_Renderer *renderer, int x, int y)
{
    int r = y;
    int c = x;
    const int scale = 2;
    for (int i = 0; i < deck.count; i++) {
        r += i + (GLYPH_H*scale);
        switch (deck.card[i]) {
            case rune_type_card_fire:
                render_card_in_deck(renderer, c, r, 0, 0, scale);
                break;
            case rune_type_card_ice:
                render_card_in_deck(renderer, c, r, 1, 0, scale);
                break;
            case rune_type_card_medical:
                render_card_in_deck(renderer, c, r, 2, 0, scale);
                break;
            case rune_type_card_slime:
                render_card_in_deck(renderer, c, r, 3, 0, scale);
                break;
            case rune_type_card_poison:
                render_card_in_deck(renderer, c, r, 4, 0, scale);
                break;
            default:
                break;
        }
    }
}
static int handle_keyup(int k)
{
    switch(k) {
    case SDLK_TAB:
        view_scale = 4.0;
        break;
    case SDLK_UP:
        input.up = 0;
        break;
    case SDLK_DOWN:
        input.down = 0;
        break;
    case SDLK_RIGHT:
        input.right = 0;
        break;
    case SDLK_LEFT:
        input.left = 0;
        break;
    }

    return 0;
}

static int handle_keydown(int k)
{

    switch(k) {
    case SDLK_0:
        break;
    case SDLK_9:
        break;
    case SDLK_ESCAPE:
        return -1;
        break;
    case SDLK_BACKSPACE:
    case SDLK_RETURN:
    case SDLK_TAB:
        view_scale = 2.0;
        break;
    case SDLK_DELETE:
        break;
    case SDLK_UP:
        input.up = 1;
        break;
    case SDLK_DOWN:
        input.down = 1;
        break;
    case SDLK_RIGHT:
        input.right = 1;
        break;
    case SDLK_LEFT:
        input.left = 1;
        break;
    default:
        break;
    }
    return 0;
}

static int handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                redraw = 1;
            }
            break;
        case SDL_QUIT:
            return -1;
        case SDL_KEYDOWN:
            return handle_keydown(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            return handle_keyup(event.key.keysym.sym);
            break;
        default:
            break;
        }
    }
    return 0;
}

static int render_console(SDL_Renderer *renderer, map_param)
{
    int i = 0;
    int r = 0;
    int c = 0;

    SDL_SetRenderTarget(renderer, gpu_texture);
    SDL_RenderClear(renderer);

    if (1 || redraw) {
        redraw = 0;
       
        // Clear backbuffer texture

        for(i = 0; i < TERM_W*TERM_H; i++) {
            r = i / TERM_W;
            c = i % TERM_W;
            render_rune(renderer, &map[r][c], r, c, 0);
        }
    }

    return 0;
}

SDL_Renderer* init_sdl()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Window *window = SDL_CreateWindow("vga rogue", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    SDL_Surface *font_surface = make_font();
    //SDL_Surface *font_surface = SDL_LoadBMP("df_font.bmp");
    SDL_SetColorKey(font_surface, SDL_TRUE, SDL_MapRGB(font_surface->format, 0xff, 0x00, 0xff ));
    font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
    SDL_FreeSurface(font_surface);

    SDL_Surface *card_surface = SDL_LoadBMP("cards.bmp");
    //SDL_SetColorKey(font_surface, SDL_TRUE, SDL_MapRGB(font_surface->format, 0xff, 0x00, 0xff ));
    card_texture = SDL_CreateTextureFromSurface(renderer, card_surface);
    SDL_FreeSurface(card_surface);

    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);
    gpu_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, W+500, H+500);

    SDL_RenderSetLogicalSize(renderer, W, H);
    /*
    back_surface = SDL_CreateRGBSurface(0, W, H, 32, rmask, gmask, bmask, amask);
    SDL_SetSurfaceBlendMode(back_surface, SDL_BLENDMODE_NONE);
    */

    return renderer;
}

static void entity_add(int r, int c, enum rune_type_enum type)
{
    if (entity_list.count < ENTITY_MAX) {
        int i = entity_list.count;
        e(i).id = type == rune_type_player ? 0 : 0xffff - i;
        e(i).r = r;
        e(i).c = c;
        e(i).active = 1;
        e(i).rune = rune_get_default(type);
        entity_list.count++;
    }
}

static void new_level(map_param)
{
    int r;
    int c;
    static int ascii_map[MAP_ROWS][MAP_COLS];

    entity_list.count = 0;
    entity_add(0, 0, rune_type_player);
    deck.count = 0;

    generate(ascii_map);

    int num_enemies = 0;
    for (r = 0; r < MAP_ROWS; r++) {
        for (c = 0; c < MAP_COLS; c++) {
            switch(ascii_map[r][c]) {
                case 'F':
                    map[r][c] = rune_get_default(rune_type_card_fire);
                    break;
                case 'I':
                    map[r][c] = rune_get_default(rune_type_card_ice);
                    break;
                case 'S':
                    map[r][c] = rune_get_default(rune_type_card_slime);
                    break;
                case 'M':
                    map[r][c] = rune_get_default(rune_type_card_medical);
                    break;
                case 'P':
                    map[r][c] = rune_get_default(rune_type_card_poison);
                    break;
                case '@':
                    map[r][c] = rune_get_default(rune_type_floor);
                    e(0).r = r;
                    e(0).c = c;
                    e(0).rune = rune_get_default(rune_type_player);
                    break;
                case 'B':
                    map[r][c] = rune_get_default(rune_type_floor);
                    entity_add(r, c, rune_type_bandit);
                    num_enemies++;
                    break;
                case 'X':
                    map[r][c] = rune_get_default(rune_type_floor);
                    entity_add(r, c, rune_type_spider);
                    num_enemies++;
                    break;
                case '$':
                    map[r][c] = rune_get_default(rune_type_floor);
                    break;
                case '.':
                    map[r][c] = rune_get_default(rune_type_floor);
                    break;
                case '<':
                    map[r][c] = rune_get_default(rune_type_stairs);
                    break;
                case '#':
                case '!':
                    map[r][c] = rune_get_default(rune_type_wall);
                    break;
                case '+':
                case '\'':
                    map[r][c] = rune_get_default(rune_type_door);
                    break;
                case ' ':
                    map[r][c] = rune_get_default(rune_type_empty);
                    break;
                default:
                    map[r][c] = rune_get_default(rune_type_floor);
                    break;
            }
        }
    }
}

static void player_tick(map_param, struct entity *e)
{
    int r = e(0).r;
    int c = e(0).c;
    int new_r = r;
    int new_c = c;

    if (input.left) {
        new_c--;
    }
    if (input.right) {
        new_c++;
    }
    if (input.up) {
        new_r--;
    }
    if (input.down) {
        new_r++;
    }

    if (r != new_r || c != new_c) {
        if (map[new_r][c].blocking) {
            new_r = r;
        }
        if (map[r][new_c].blocking) {
            new_c = c;
        }
        if (!map[new_r][new_c].blocking) {
            e(0).r = new_r;
            e(0).c = new_c;
            r = new_r;
            c = new_c;
        }
    }

    switch (map[r][c].type) {
        case rune_type_stairs:
            ++level;
            new_level(map);
            break;
        case rune_type_door:
            if (map[r][c].i == 0) {
                map[r][c].i = (map[r][c].i + 1) % (sizeof(map[r][c].ch) / sizeof(char));
            }
            break;
        case rune_type_card_fire:
        case rune_type_card_ice:
        case rune_type_card_medical:
        case rune_type_card_slime:
        case rune_type_card_poison:
            if (deck_add_card(map[r][c].type)) {
                map[r][c] = rune_get_default(rune_type_floor);
            }
        default:
            break;
    }
}
static void enemy_tick(map_param, struct entity *e)
{
    int i;
    int r = e(0).r;
    int c = e(0).c;

    int er = e->r;
    int ec = e->c;

    if (er > r) {
        e->r--;
    } else if (er < r) {
        e->r++;
    }

    if (ec > c) {
        e->c--;
    }
    else if (ec < c) {
        e->c++;
    }

    if (map[e->r][ec].blocking) {
        e->r = er;
    }

    if (map[er][e->c].blocking) {
        e->c = ec;
    }

    for (i = 0; i < entity_list.count; i++) {
        if (e(i).id == e->id) continue;

        if (e(i).r == e->r && e(i).c == e->c) {
            e->r = er;
            e->c = ec;
        }
    }

    if (e->r == r && e->c == c) {
        e->active = 0;
    }
}

static void entity_tick(map_param, struct entity *e)
{
    switch(e->rune.type) {
    case rune_type_player:
        player_tick(map, e);
        break;
    case rune_type_spider:
    case rune_type_bandit:
        enemy_tick(map, e);
        break;
    case rune_type_floor:
    case rune_type_door:
    case rune_type_wall:
    case rune_type_stairs:
    case rune_type_empty:
    case rune_type_count:
    default:
        break;
    }
}

static void main_loop_body(SDL_Renderer *renderer, map_param)
{
    int i, r, c;
    int tick = input.up || input.down || input.left || input.right;
    Uint32 current_millis = SDL_GetTicks();
    static Uint32 last_millis = 0;
    static Uint32 last_tick_millis = 0;

    if((tick = tick && (current_millis - last_tick_millis > 120))) {
        last_tick_millis = SDL_GetTicks();
    }

    render_console(renderer, map);
    for (i = 0; i < MAP_ROWS*MAP_COLS; i++) {
        r = i / MAP_COLS;
        c = i % MAP_COLS;
        render_rune(renderer, &map[r][c], r, c, 0);
    }

    for(i = 0; i < entity_list.count; i++) {
        if (tick) {
            entity_tick(map, &e(i));
        }

        if (e(i).active) {
            render_entity(renderer, &e(i), 0);
        }
    }
    SDL_SetRenderTarget(renderer, NULL);

    double half_scale = view_scale / 2.0;
    double gpu_x = CLAMP(e(0).c * GLYPH_W - (double)W/view_scale, 0, GPU_W);
    double gpu_y = CLAMP(e(0).r * GLYPH_H - (double)H/view_scale, 0, GPU_H);

    SDL_Rect gpu_src = {.w = W/half_scale, .h = H/half_scale, .x = (int)gpu_x, .y = (int)gpu_y };
    SDL_Rect gpu_dst= {.w = W, .h = H, .x = 0, .y = 0};
    SDL_RenderCopy(renderer, gpu_texture, &gpu_src, &gpu_dst);
    render_deck(renderer, W - 40, 10);
    
    SDL_RenderPresent(renderer);

    while (current_millis < last_millis + 16) {
        current_millis = SDL_GetTicks();
        SDL_Delay(1);
    }
    last_millis = current_millis;
}

int main()
{
    struct rune map[MAP_ROWS][MAP_COLS];
    SDL_Renderer *renderer = init_sdl();
    
    srand(time(0));
    new_level(map);

    while (handle_events() != -1) {
        main_loop_body(renderer, map);
    }

    die(0);
}

