#ifndef _RUNE_H_
#define _RUNE_H_

enum rune_type_enum {
   rune_type_floor = 0,
   rune_type_door,
   rune_type_wall,
   rune_type_player,
   rune_type_stairs,
   rune_type_bandit,
   rune_type_spider,

   rune_type_card_fire,
   rune_type_card_ice,
   rune_type_card_medical,
   rune_type_card_slime,
   rune_type_card_poison,

   rune_type_empty,
   rune_type_count,
};

struct rune {
    enum rune_type_enum type;
    char ch[4];
    int i;
    int blocking;
    int bg;
    int fg;
};

static struct rune rune_default[rune_type_count+1] = {
    {
        .type = rune_type_floor,
        .ch = { '.', '.', ' ', ' ' },
        .i = 0,
        .blocking = 0,
        .bg = 0,
        .fg = 7,
    },
    {
        .type = rune_type_door,
        .ch = { '+', '\'', '+', '\'' },
        .i = 0,
        .blocking = 0,
        .bg = 0,
        .fg = 7,
    },
    {
        .type = rune_type_wall,
        .ch = { '#', '#', '#', '#' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 7,
    },
    {
        .type = rune_type_player,
        .ch = { '@', '@', '@', '@' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 11,
    },
    {
        .type = rune_type_stairs,
        .ch = { '<', '<', '<', '<' },
        .i = 0,
        .blocking = 0,
        .bg = 0,
        .fg = -1,
    },
    {
        .type = rune_type_bandit,
        .ch = { 'B', '!', 'B', '!' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 13,
    },
    {
        .type = rune_type_spider,
        .ch = { 'X', '!', 'X', '!' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 10,
    },
    {
        .type = rune_type_card_fire,
        .ch = { 'F', 'F', 'F', 'F' },
        .i = 0,
        .blocking = 0,
        .bg = -1,
        .fg = -1,
    },
    {
        .type = rune_type_card_ice,
        .ch = { 'I', 'I', 'I', 'I' },
        .i = 0,
        .blocking = 0,
        .bg = -1,
        .fg = -1,
    },
    {
        .type = rune_type_card_medical,
        .ch = { 'M', 'M', 'M', 'M' },
        .i = 0,
        .blocking = 0,
        .bg = -1,
        .fg = -1,
    },
    {
        .type = rune_type_card_slime,
        .ch = { 'S', 'S', 'S', 'S' },
        .i = 0,
        .blocking = 0,
        .bg = -1,
        .fg = -1,
    },
    {
        .type = rune_type_card_poison,
        .ch = { 'P', 'P', 'P', 'P' },
        .i = 0,
        .blocking = 0,
        .bg = -1,
        .fg = -1,
    },
    {
        .type = rune_type_empty,
        .ch = { ' ', ' ', ' ', ' ' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 0,
    },
    {
        .type = rune_type_count,
        .ch = { '?', '?', '?', '?' },
        .i = 0,
        .blocking = 1,
        .bg = 0,
        .fg = 0,
    },

};

struct rune rune_get_default(enum rune_type_enum rt)
{
    return rune_default[rt];
}

#endif

