#ifndef BOARD_ARRAY_H
#define BOARD_ARRAY_H

struct GameBoard {
  int **data;
  int width;
  int height;
};

struct Index {
  int y;
  int x;
};

extern void GameBoard_init(GameBoard *board, int width, int height);
extern void GameBoard_deinit(GameBoard *board);

extern Index GameBoard_first(GameBoard *board);
extern int *GameBoard_index(GameBoard *board, Index idx);

#endif
