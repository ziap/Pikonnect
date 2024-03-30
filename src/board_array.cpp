#include "board_array.h"
#include <stdlib.h>

void GameBoard_init(GameBoard *board, int width, int height) {
  board->width = width;
  board->height = height;
  board->data = (int**)malloc((height + 4) * sizeof(int*));

  for (int i = 0; i < height + 4; ++i) {
    board->data[i] = (int*)malloc((width + 4) * sizeof(int));
  }

  for (int i = 0; i < height + 4; ++i) {
    board->data[i][0] = -1;
    board->data[i][width + 3] = -1;
  }

  for (int i = 1; i < width + 3; ++i) {
    board->data[0][i] = -1;
    board->data[height + 3][i] = -1;
  }

  for (int i = 1; i < height + 3; ++i) {
    board->data[i][1] = 0;
    board->data[i][width + 2] = 0;
  }

  for (int i = 2; i < width + 2; ++i) {
    board->data[1][i] = 0;
    board->data[height + 2][i] = 0;
  }
}

void GameBoard_deinit(GameBoard *board) {
  for (int i = 0; i < board->height + 4; ++i) {
    free(board->data[i]);
  }

  free(board->data);
}

int *GameBoard_index(GameBoard *board, Index idx) {
  return &board->data[idx.y + 2][idx.x + 2];
}
