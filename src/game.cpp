#include "game.h"

void Game_init(Game *game) {
  game->current_user = nullptr;
  UserTable_load(&game->users);
}

void Game_deinit(Game *game) {
  UserTable_save(&game->users);
}

Index Index_step(Index current, Dir dir) {
  Index next = next_index[dir];
  return {current.y + next.y, current.x + next.x};
}
