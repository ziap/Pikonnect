#include "board_array.h"
#include <stdlib.h>

int *GameBoard_index(GameBoard board, Index idx) {
  return board.data + ((idx.y + 2) * (board.width + 4) + (idx.x + 2));
}

void GameBoard_init(GameBoard *board, int width, int height) {
  board->width = width;
  board->height = height;
  board->data = (int*)malloc((height + 4) * (width + 4) * sizeof(int*));

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
}

void GameBoard_deinit(GameBoard *board) {
  free(board->data);
}
