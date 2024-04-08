#include "search.h"

#include <stdlib.h>
#include <string.h>

#include "board_array.h"

// Valid turing directions
// Stored in an array for later iteration
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

static const int INIT_CAP = 1024;
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

// Expand a search vertex and push them to the queue
static void push_queue(Queue *q, Vertex vertex) {
  Index index_current = vertex.path.data[vertex.path.len - 1];

  // Continue going in the current direction by replacing the last segment in
  // the path
  Vertex vertex_tmp = vertex;
  Index index_next = Index_step(index_current, vertex.dir);
  vertex_tmp.path.data[vertex_tmp.path.len - 1] = index_next;
  Queue_enqueue(q, vertex_tmp);

  // The path is less than 3 segments (4 pivots)
  // Rotate and push another segment to the path
  if (vertex.path.len < 4) {
    for (int i = 0; i < 2; i++) {
      Vertex vertex_tmp = vertex;
      vertex_tmp.dir = next_dirs[vertex.dir][i];
      Index index_next = Index_step(index_current, next_dirs[vertex.dir][i]);
      vertex_tmp.path.data[vertex_tmp.path.len++] = index_next;
      Queue_enqueue(q, vertex_tmp);
    }
  }
}

// Expand the start position in all directions and push them to the queue
static void push_start(Queue *q, Index index_current) {
  Queue_clear(q);
  Vertex vertex;
  vertex.path.data[0] = index_current;
  vertex.path.len = 1;
  
  for (int i = 0; i < DIR_LEN; i++) {
    Vertex vertex_tmp = vertex;
    vertex_tmp.dir = (Dir)i;
    Index index_next = Index_step(index_current, vertex_tmp.dir);

    vertex_tmp.path.data[vertex_tmp.path.len++] = index_next;
    Queue_enqueue(q, vertex_tmp);
  }
}

// Check if the path matched by the user is valid
Path Search_check_path(GameBoard board, Index start, Index end, Queue *q) {
  int val_begin = board.data[start.y][start.x];
  int val_end = board.data[end.y][end.x];
  if (val_begin != val_end || val_begin == 0) return { {}, 0 };

  push_start(q, start);

  while (Vertex *top = Queue_dequeue(q)) {
    Index index_current = top->path.data[top->path.len - 1];
    if (index_current.y == end.y && index_current.x == end.x) {
      // The endpoints are connected by a path with less than 3 segments
      return top->path;
    }
    if (board.data[index_current.y][index_current.x] == 0) {
      push_queue(q, *top);
    }
  }

  return { {}, 0 };
}

// Find a valid move to suggest to the player
Path Search_suggest_move(GameBoard board, Queue *q) {
  Index start;
  // Iterate over all tiles
  for (start.y = 0; start.y < board.height; ++start.y) {
    for (start.x = 0; start.x < board.width; ++start.x) {

      int target = board.data[start.y][start.x];
      if (target) {

        // An empty tile is found, search for another tile with the same value
        push_start(q, start);

        while (Vertex *top = Queue_dequeue(q)) {
          Index current = top->path.data[top->path.len - 1];
          int val = board.data[current.y][current.x];
          if (val == target) return top->path;
          if (val == 0) push_queue(q, *top);
        }
      }
    }
  }

  return { {}, 0 };
}
