#include "game.h"

Game Game_create() {
  Game game;
  game.scene = SCENE_LOGIN;
  LoginMenu_init(&game.login_menu);

  return game;
}

void Game_deinit(Game *game) {
  LoginMenu_deinit(&game->login_menu);
}

