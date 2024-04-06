#ifndef SEARCH_H
#define SEARCH_H

#include "board_array.h"

struct Path {
  Index data[4];
  int len;
};

struct Vertex {
  Path path;
  Dir dir;
};

struct Queue {
  Vertex *data;
  uint32_t head;
  uint32_t tail;

  uint32_t len;
  uint32_t cap;
};

void Queue_clear(Queue *q);
void Queue_init(Queue *q);
void Queue_deinit(Queue *q);
void Queue_enqueue(Queue *q, Vertex v);
Vertex *Queue_dequeue(Queue *q);

Path Search_valid_path(GameBoard board, Index index_begin, Index index_end, Queue *q);
Path Search_suggest_move(GameBoard board, Queue *q);

#endif
