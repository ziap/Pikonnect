#include "search.h"
#include "board_array.h"
#include <queue>

const Dir next_dirs[DIR_LEN][2] = {
    {DIR_LEFT, DIR_RIGHT},
    {DIR_UP, DIR_DOWN},
    {DIR_LEFT, DIR_RIGHT},
    {DIR_UP, DIR_DOWN}
};

struct Vertex {
    Path path;
    Dir dir;
};

void push_queue(std::queue<Vertex> *q, Vertex vertex) {
    Index index_current = vertex.path.data[vertex.path.len - 1];
    Vertex vertex_tmp = vertex;
    Index index_next = Index_step(index_current, vertex.dir);
    vertex_tmp.path.data[vertex_tmp.path.len - 1] = index_next;
    q->push(vertex_tmp);

    if (vertex.path.len < 4) {
        for (int i = 0; i < 2; i++) {
            Vertex vertex_tmp = vertex;
            vertex_tmp.dir = next_dirs[vertex.dir][i];
            Index index_next = Index_step(index_current, next_dirs[vertex.dir][i]);
            vertex_tmp.path.data[vertex_tmp.path.len] = index_next;
            vertex_tmp.path.len++;
            q->push(vertex_tmp);
        }
    }
}

void push_start(std::queue<Vertex> *q, Index index_current) {
    Vertex vertex;
    vertex.path.data[0] = index_current;
    vertex.path.len = 1;
    for (int i = 0; i < DIR_LEN; i++) {
        Vertex vertex_tmp = vertex;
        vertex_tmp.dir = (Dir)i;
        Index index_next = Index_step(index_current, vertex_tmp.dir);

        vertex_tmp.path.data[vertex_tmp.path.len] = index_next;
        vertex_tmp.path.len++;
        q->push(vertex_tmp);
    }
}

Path Search_valid_path(GameBoard board, Index index_begin, Index index_end) {
    Vertex vertex;
    int val_begin = *GameBoard_index(board, index_begin);
    int val_end = *GameBoard_index(board, index_end);
    if (val_begin != val_end || val_begin == 0) {
        vertex.path.len = 0;
        return vertex.path;
    }
    std::queue<Vertex> q;
    push_start(&q, index_begin);
    while (!q.empty()) {
        vertex = q.front();
        q.pop();

        Index index_current = vertex.path.data[vertex.path.len - 1];
        int val = *(GameBoard_index(board, index_current));
        if (index_current.y == index_end.y && index_current.x == index_end.x) {
          return vertex.path;
        }
        if (val == 0) push_queue(&q, vertex);
    }

    vertex.path.len = 0;
    return vertex.path;
}

Path find_path(GameBoard board, Index index_begin) {
    std::queue<Vertex> q;
    Vertex vertex;
    push_start(&q, index_begin);
    int target = *(GameBoard_index(board, index_begin));
    while (!q.empty()) {
        vertex = q.front();
        q.pop();

        Index index_current = vertex.path.data[vertex.path.len - 1];
        int val = *(GameBoard_index(board, index_current));
        if (val == target) return vertex.path;
        if (val == 0) push_queue(&q, vertex);
    }

    vertex.path.len = 0;
    return vertex.path;
}

Path Search_suggest_move(GameBoard board) {
    for (int i = 0; i < board.height; i++) {
        for (int j = 0; j < board.width; j++) {
            Index index_begin = { i, j };

            if (*GameBoard_index(board, index_begin) != 0) {
                Path path_find = find_path(board, index_begin);
                if (path_find.len != 0) return path_find;
            }
        }
    }
    Path path_find;
    path_find.len = 0;
    return path_find;
}
