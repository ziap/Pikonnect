#include "scene_leaderboard.h"
#include <raylib.h>

Scene Scene_leaderboard_update(Game *game, float dt) {
  (void)dt;

  Scene_draw_header(game, "Leaderboard");

  if (IsKeyPressed(KEY_Q) || IsKeyPressedRepeat(KEY_Q)) return SCENE_HOME;
  return SCENE_LEADERBOARD;
};
