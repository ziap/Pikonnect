#include "scene.h"
#include "config.h"
#include <raylib.h>
#include <stdio.h>

Scene Scene_main_update(Game *game, float dt) {
  (void)game;
  (void)dt;

  const int TITLE_SIZE = 72;
  const char title[] = "Main menu";
  DrawText(title, (SCREEN_WIDTH - MeasureText(title, TITLE_SIZE)) / 2, 150, TITLE_SIZE, DARKBLUE);
  char msg[1024];
  snprintf(msg, sizeof(msg), "Logged in as %s", game->current_user->name);
  DrawText(msg, (SCREEN_WIDTH - MeasureText(msg, 24)) / 2, 320, 24, DARKGRAY);

  return SCENE_MAIN;
}

