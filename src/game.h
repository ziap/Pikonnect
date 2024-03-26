#ifndef GAME_H
#define GAME_H

#include "login.h"

enum Scene {
  SCENE_LOGIN,
  SCENE_MAIN,
  SCENE_GAME,
  SCENE_LEN
};

struct Game {
  Scene scene;
  LoginMenu login_menu;
};

extern Game Game_create(void);
extern void Game_deinit(Game *game);

#endif
