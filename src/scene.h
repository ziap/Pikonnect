#ifndef SCENE_H
#define SCENE_H

#include "game.h"

enum Scene {
  SCENE_LOGIN,
  SCENE_MAIN,
  SCENE_GAME,
  SCENE_LEN
};

extern Scene Scene_login_update(Game *game, float dt);
extern Scene Scene_main_update(Game *game, float dt);
extern Scene Scene_game_update(Game *game, float dt);

Scene (*const Scene_update_functions[SCENE_LEN])(Game *game, float dt) = {
  Scene_login_update,
  Scene_main_update,
  Scene_game_update
};

#endif
