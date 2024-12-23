#ifndef BOARD_ARRAY_H
#define BOARD_ARRAY_H

#include <stdbool.h>
#include <stdint.h>

typedef int8_t Tile;

typedef struct {
  Tile **data;
  int width;
  int height;
} GameBoard;

typedef struct {
  int y;
  int x;
} Index;

typedef enum {
  DIR_UP,
  DIR_LEFT,
  DIR_DOWN,
  DIR_RIGHT,
  DIR_LEN
} Dir;

extern void GameBoard_init(GameBoard *board, int width, int height,
                           int num_classes, uint64_t *random_state);
extern void GameBoard_deinit(GameBoard *board);

extern Index GameBoard_first(GameBoard *board);

extern bool GameBoard_remove_row(GameBoard *board, int y);
extern bool GameBoard_remove_col(GameBoard *board, int x);

extern Index Index_step(Index index_current, Dir dir_next);

#endif
