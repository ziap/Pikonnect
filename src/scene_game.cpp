#include "scene.h"
#include "config.h"
#include <raylib.h>

Scene Scene_game_update(Game *game, float dt) {
  (void)game;
  (void)dt;

  const int TITLE_SIZE = 72;
  const char title[] = "Game menu";
  DrawText(title, (SCREEN_WIDTH - MeasureText(title, TITLE_SIZE)) / 2, 150, TITLE_SIZE, DARKBLUE);

  return SCENE_GAME;
}

