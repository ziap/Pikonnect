#include "scene_leaderboard.h"
#include <raylib.h>

Scene Scene_leaderboard_update(Game *game, float dt) {
  (void)dt;

  Scene_draw_header(game, "Leaderboard");
  return SCENE_LEADERBOARD;
};
