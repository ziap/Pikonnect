#include "search.h"
#include "board_array.h"
#include <stdlib.h>
#include <string.h>

const Dir next_dirs[DIR_LEN][2] = {
  {DIR_LEFT, DIR_RIGHT},
  {DIR_UP, DIR_DOWN},
  {DIR_LEFT, DIR_RIGHT},
  {DIR_UP, DIR_DOWN}
};

void Queue_clear(Queue *q) {
  q->head = 0;
  q->tail = 0;
  q->len = 0;
}

void Queue_init(Queue *q) {
  q->data = (Vertex*)malloc(INIT_CAP * sizeof(Vertex));
  q->cap = INIT_CAP;
  Queue_clear(q);
}

void Queue_deinit(Queue *q) {
  free(q->data);
}

void Queue_enqueue(Queue *q, Vertex v) {
  if (q->len == q->cap) {
    q->data = (Vertex*)realloc(q->data, (q->cap <<= 1) * sizeof(v));
    memcpy(q->data + q->len, q->data, q->head * sizeof(v));
    q->head += q->len;
  }
  q->data[q->head] = v;
  q->head = (q->head + 1) & (q->cap - 1);
  ++q->len;
}

Vertex *Queue_dequeue(Queue *q) {
  if (q->len == 0) return nullptr;
  Vertex *v = q->data + q->tail;
  q->tail = (q->tail + 1) & (q->cap - 1);
  --q->len;
  return v;
}

void push_queue(Queue *q, Vertex vertex) {
  Index index_current = vertex.path.data[vertex.path.len - 1];
  Vertex vertex_tmp = vertex;
  Index index_next = Index_step(index_current, vertex.dir);
  vertex_tmp.path.data[vertex_tmp.path.len - 1] = index_next;
  Queue_enqueue(q, vertex_tmp);

  if (vertex.path.len < 4) {
    for (int i = 0; i < 2; i++) {
      Vertex vertex_tmp = vertex;
      vertex_tmp.dir = next_dirs[vertex.dir][i];
      Index index_next = Index_step(index_current, next_dirs[vertex.dir][i]);
      vertex_tmp.path.data[vertex_tmp.path.len] = index_next;
      vertex_tmp.path.len++;
      Queue_enqueue(q, vertex_tmp);
    }
  }
}

void push_start(Queue *q, Index index_current) {
  Vertex vertex;
  vertex.path.data[0] = index_current;
  vertex.path.len = 1;
  for (int i = 0; i < DIR_LEN; i++) {
    Vertex vertex_tmp = vertex;
    vertex_tmp.dir = (Dir)i;
    Index index_next = Index_step(index_current, vertex_tmp.dir);

    vertex_tmp.path.data[vertex_tmp.path.len] = index_next;
    vertex_tmp.path.len++;
    Queue_enqueue(q, vertex_tmp);
  }
}

Path Search_valid_path(GameBoard board, Index index_begin, Index index_end, Queue *q) {
  int val_begin = *GameBoard_index(board, index_begin);
  int val_end = *GameBoard_index(board, index_end);
  if (val_begin != val_end || val_begin == 0) {
    Vertex vertex;
    vertex.path.len = 0;
    return vertex.path;
  }

  Queue_clear(q);
  push_start(q, index_begin);

  while (Vertex *top = Queue_dequeue(q)) {
    Index index_current = top->path.data[top->path.len - 1];
    int val = *(GameBoard_index(board, index_current));
    if (index_current.y == index_end.y && index_current.x == index_end.x) {
      return top->path;
    }
    if (val == 0) push_queue(q, *top);
  }

  Vertex vertex;
  vertex.path.len = 0;
  return vertex.path;
}

Path find_path(GameBoard board, Index index_begin, Queue *q) {
  Queue_clear(q);
  push_start(q, index_begin);
  int target = *(GameBoard_index(board, index_begin));

  while (Vertex *top = Queue_dequeue(q)) {
    Index index_current = top->path.data[top->path.len - 1];
    int val = *(GameBoard_index(board, index_current));
    if (val == target) return top->path;
    if (val == 0) push_queue(q, *top);
  }

  Vertex vertex;
  vertex.path.len = 0;
  return vertex.path;
}

Path Search_suggest_move(GameBoard board, Queue *q) {
  for (int i = 0; i < board.height; i++) {
    for (int j = 0; j < board.width; j++) {
      Index index_begin = { i, j };

      if (*GameBoard_index(board, index_begin) != 0) {
        Path path_find = find_path(board, index_begin, q);
        if (path_find.len != 0) return path_find;
      }
    }
  }
  Path path_find;
  path_find.len = 0;
  return path_find;
}
