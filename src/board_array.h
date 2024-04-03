#ifndef BOARD_ARRAY_H
#define BOARD_ARRAY_H

#include <stdint.h>

typedef int8_t Tile;

struct GameBoard {
  Tile *data;
  int width;
  int height;
};

struct Index {
  int y;
  int x;
};

enum Dir {
  DIR_UP,
  DIR_LEFT,
  DIR_DOWN,
  DIR_RIGHT,
  DIR_LEN
};

static const Index next_index[DIR_LEN] = {
  {-1, 0},
  {0, -1},
  {1, 0},
  {0, 1}
};

extern void GameBoard_init(GameBoard *board, int width, int height);
extern void GameBoard_deinit(GameBoard *board);

extern Index GameBoard_first(GameBoard *board);
extern Tile *GameBoard_index(GameBoard board, Index idx);

extern bool GameBoard_remove_row(GameBoard *board, int y);
extern bool GameBoard_remove_col(GameBoard *board, int x);

extern Index Index_step(Index index_current, Dir dir_next);

#endif
