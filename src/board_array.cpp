#include "board_array.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

static const Index next_index[DIR_LEN] = {
  {-1, 0},
  {0, -1},
  {1, 0},
  {0, 1}
};

Tile *GameBoard_index(GameBoard board, Index idx) {
  return board.data + ((idx.y + 2) * (board.width + 4) + (idx.x + 2));
}

void GameBoard_init(GameBoard *board, int width, int height, int num_classes, uint64_t *random_state) {
  board->width = width;
  board->height = height;
  board->data = (Tile*)malloc((height + 4) * (width + 4) * sizeof(Tile));

  for (int i = -2; i < height + 2; ++i) {
    *GameBoard_index(*board, {i, -2}) = -1;
    *GameBoard_index(*board, {i, width + 1}) = -1;
  }

  for (int i = -1; i < width + 1; ++i) {
    *GameBoard_index(*board, {-2, i}) = -1;
    *GameBoard_index(*board, {height + 1, i}) = -1;
  }

  for (int i = -1; i < height + 1; ++i) {
    *GameBoard_index(*board, {i, -1}) = 0;
    *GameBoard_index(*board, {i, width}) = 0;
  }

  for (int i = 0; i < width; ++i) {
    *GameBoard_index(*board, {-1, i}) = 0;
    *GameBoard_index(*board, {height, i}) = 0;
  }

  int idx = pcg32_bounded(random_state, num_classes) * 2;
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      Tile *tile = GameBoard_index(*board, {i, j});
      *tile = (idx++ / 2) % num_classes + 1;
    }
  }

  int total = height * width;
  for (int i = 0; i < total - 1; ++i) {
    int j = pcg32_bounded(random_state, total - i) + i;
    Tile *p1 = GameBoard_index(*board, {i / width, i % width});
    Tile *p2 = GameBoard_index(*board, {j / width, j % width});

    int t = *p1;
    *p1 = *p2;
    *p2 = t;
  }
}

void GameBoard_deinit(GameBoard *board) {
  free(board->data);
}

Index Index_step(Index current, Dir dir) {
  Index next = next_index[dir];
  return {current.y + next.y, current.x + next.x};
}

bool GameBoard_remove_row(GameBoard *board, int y) {
  for (int i = 0; i < board->width; ++i) {
    if (*GameBoard_index(*board, {y, i})) return false;
  }

  int w = board->width + 4;
  Tile *row = board->data + (y + 2) * w;
  memmove(row, row + w, (board->height - y + 1) * w * sizeof(Tile));
  --board->height;
  return true;
}

bool GameBoard_remove_col(GameBoard *board, int x) {
  for (int i = 0; i < board->height; ++i) {
    if (*GameBoard_index(*board, {i, x})) return false;
  }

  int w = board->width + 4;
  int move_size = (w - 1) * sizeof(Tile);
  Tile *p1 = board->data + x + 2;
  Tile *p2 = p1 + 1;

  for (int i = 1; i < board->height + 4; ++i) {
    memmove(p1, p2, move_size);
    p1 += w - 1;
    p2 += w;
  }

  memmove(p1, p2, (w - (x + 3)) * sizeof(Tile));

  --board->width;
  return true;
}
