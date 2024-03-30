#ifndef SCENE_GAME_H
#define SCENE_GAME_H

#include "scene.h"

extern void Scene_game_load(Game *game);
extern Scene Scene_game_update(Game *game, float dt);
extern void Scene_game_unload(Game *game);

#endif
