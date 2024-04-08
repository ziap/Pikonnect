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

void GameBoard_init(GameBoard *board, int width, int height, int num_classes, uint64_t *random_state) {
  board->width = width;
  board->height = height;

  int h = height + 4;
  int w = width + 4;

  void *buf = malloc(h * sizeof(Tile*) + h * w * sizeof(Tile));
  board->data = (Tile**)buf + 2;
  board->data[-2] = (Tile*)(board->data + height + 2) + 2;
  for (int i = -1; i < height + 2; ++i) board->data[i] = board->data[i - 1] + w;

  for (int i = -2; i < height + 2; ++i) {
    board->data[i][-2] = -1;
    board->data[i][width + 1] = -1;
  }

  for (int i = -1; i < width + 1; ++i) {
    board->data[-2][i] = -1;
    board->data[height + 1][i] = -1;
  }

  for (int i = -1; i < height + 1; ++i) {
    board->data[i][-1] = 0;
    board->data[i][width] = 0;
  }

  for (int i = 0; i < width; ++i) {
    board->data[-1][i] = 0;
    board->data[height][i] = 0;
  }

  int idx = pcg32_bounded(random_state, num_classes) * 2;
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      board->data[i][j] = (idx++ / 2) % num_classes + 1;
    }
  }

  int total = height * width;
  for (int i = 0; i < total - 1; ++i) {
    int j = pcg32_bounded(random_state, total - i) + i;
    Tile *p1 = &board->data[i / width][i % width];
    Tile *p2 = &board->data[j / width][j % width];

    int t = *p1;
    *p1 = *p2;
    *p2 = t;
  }
}

void GameBoard_deinit(GameBoard *board) {
  free(board->data - 2);
}

Index Index_step(Index current, Dir dir) {
  Index next = next_index[dir];
  return {current.y + next.y, current.x + next.x};
}

bool GameBoard_remove_row(GameBoard *board, int y) {
  for (int i = 0; i < board->width; ++i) {
    if (board->data[y][i]) return false;
  }
  
  memmove(board->data + y, board->data + y + 1, (board->height - y + 1) * sizeof(Tile*));

  --board->height;
  return true;
}

bool GameBoard_remove_col(GameBoard *board, int x) {
  for (int i = 0; i < board->height; ++i) {
    if (board->data[i][x]) return false;
  }

  for (int i = -2; i < board->height + 2; ++i) {
    memmove(board->data[i] + x, board->data[i] + x + 1, (board->width - x + 1) * sizeof(Tile));
  }

  --board->width;
  return true;
}
