#ifndef _GENERATE_H
#define _GENERATE_H

#include "defines.h"

/***************************/
/* Modified from this gist */
/***********************************************************************/
/* https://gist.github.com/munificent/b1bcd969063da3e6c298be070a22b604 */
/***********************************************************************/

/* Robert Nystrom @munificentbob for Ginny 2008-2019 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

const int PLAYER   = '@';
const int BANDIT   = 'B';
const int SPIDER   = 'X';
const int TREASURE = '$';
const int ROCK     = ' ';
const int CORNER   = '!';
const int STAIRS   = '<';
const int WALL     = '#';
const int FLOOR    = '.';
const int DOOR1    = '+';
const int DOOR2    = '\'';

void cave(int map[MAP_ROWS][MAP_COLS], int start)
{
  int width = (rand() % 14) + 4;
  int height = (rand() % 8) + 4;
  int left = (rand() % (MAP_COLS - width - 2)) + 1;
  int top = (rand() % (MAP_ROWS - height - 2)) + 1;
  int x, y;

  for (y = top - 1; y < top + height + 2; y++)
    for (x = left - 1; x < left + width + 2; x++)
      if (map[y][x] == FLOOR)
        return;

  int doors = 0;
  int door_x, door_y;

  if (!start) {
    for (y = top - 1; y < top + height + 2; y++)
      for (x = left - 1; x < left + width + 2; x++) {
        int s = x < left || x > left + width;
        int t = y < top || y > top + height;
        if (s ^ t && map[y][x] == WALL) {
          doors++;
          if (rand() % doors == 0)
            door_x = x, door_y = y;
        }
      }

    if (doors == 0)
      return;
  }

  for (y = top - 1; y < top + height + 2; y++)
    for (x = left - 1; x < left + width + 2; x++) {
      int s = x < left || x > left + width;
      int t = y < top || y > top + height;
      map[y][x] = s && t ? CORNER : s ^ t ? WALL : FLOOR;
    }

  if (doors > 0)
    map[door_y][door_x] = (rand() % 2) ? DOOR2 : DOOR1;

  if (start) {
    y = rand() % height;    
    x = rand() % width;    
    map[y + top][x + left] = PLAYER;
  }


  for (int j = 0; !start && j < (rand() % 6) + 1; j++) {
    int y = rand() % height;    
    int x = rand() % width;    
    //if (map[y + top][x + left] == STAIRS) { continue; }
    switch(rand() % 8) {
    case 0:
      map[y + top][x + left] = TREASURE;
      break;
    case 1:
      map[y + top][x + left] = SPIDER;
      break;
    case 2:
      map[y + top][x + left] = BANDIT;
      break;
    case 3:
      map[y + top][x + left] = 'F';
      break;
    case 4:
      map[y + top][x + left] = 'I';
      break;
    case 5:
      map[y + top][x + left] = 'S';
      break;
    case 6:
      map[y + top][x + left] = 'M';
      break;
    case 7:
      map[y + top][x + left] = 'P';
      break;
    }
  }
}

void generate(int map[MAP_ROWS][MAP_COLS])
{
  int x, y;
  srand((int)time(NULL));
  for (int y = 0; y < MAP_ROWS; y++)
    for (int x = 0; x < MAP_COLS; x++)
      map[y][x] = ROCK;

  for (int j = 0; j < 1000; j++)
    cave(map, j == 0);

  do {
    y = rand() % MAP_ROWS;    
    x = rand() % MAP_COLS;    
  } while (map[y][x] == WALL || map[y][x] == ROCK);
  map[y][x] = STAIRS;

  for (int y = 0; y < MAP_ROWS; y++)
    for (int x = 0; x < MAP_COLS; x++) {
      int c = map[y][x];
      putchar(c == CORNER ? WALL : c);
      if (x == MAP_COLS - 1)
        printf("\n");
    }
}

#endif
