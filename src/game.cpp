#include "game.h"

Game Game_create() {
  Game game;
  game.current_user = nullptr;
  UserTable_load(&game.users);
  LoginMenu_init(&game.login_menu);

  return game;
}

void Game_deinit(Game *game) {
  LoginMenu_deinit(&game->login_menu);
  UserTable_save(&game->users);
}

