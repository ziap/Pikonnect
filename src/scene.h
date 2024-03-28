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

extern void Scene_draw_header(Game *game, const char *menu_name);
extern void Scene_init(Scene scene, Game *game);
extern Scene Scene_update(Scene scene, Game *game, float dt);
extern void Scene_deinit(Scene scene, Game *game);

#endif
