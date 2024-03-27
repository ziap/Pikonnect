#ifndef SCENE_H
#define SCENE_H

#include "game.h"

enum Scene {
  SCENE_LOGIN,
  SCENE_HOME,
  SCENE_LEVELS,
  SCENE_GAME,
  SCENE_LEADERBOARD,
  SCENE_LEN
};

extern float easing_cubic(float);
extern void Scene_draw_header(Game *game, const char *menu_name);

extern Scene Scene_login_update(Game *game, float dt);
extern Scene Scene_home_update(Game *game, float dt);
extern Scene Scene_levels_update(Game *game, float dt);
extern Scene Scene_game_update(Game *game, float dt);
extern Scene Scene_leaderboard_update(Game *game, float dt);

Scene (*const Scene_update_functions[SCENE_LEN])(Game *game, float dt) = {
  Scene_login_update,
  Scene_home_update,
  Scene_levels_update,
  Scene_game_update,
  Scene_leaderboard_update
};

#endif
