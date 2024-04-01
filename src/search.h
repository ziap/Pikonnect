#ifndef SEARCH_H
#define SEARCH_H

#include "game.h"
#include "board_array.h"

struct Path {
    Index data[4];
    int len;
};

Path Search_valid_path(GameBoard board, Index index_begin, Index index_end);
Path Search_suggest_move(GameBoard board);

#endif
