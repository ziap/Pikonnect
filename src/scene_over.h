#ifndef SCENE_OVER_H
#define SCENE_OVER_H

#include "scene.h"

extern void Scene_won_load(Game *game);
extern Scene Scene_won_update(Game *game, float dt);

extern void Scene_lost_load(Game *game);
extern Scene Scene_lost_update(Game *game, float dt);

#endif
