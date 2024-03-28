#include "scene.h"
#include "config.h"

#include <raylib.h>
#include <stdio.h>

void Scene_draw_header(Game *game, const char *menu_name) {
  DrawRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, LIGHTGRAY);
  DrawText(menu_name, 32, 32, 32, BLACK);

  char msg[1024];
  snprintf(msg, sizeof(msg), "Logged in as: %s", game->current_user->name);
  DrawText(msg, SCREEN_WIDTH - MeasureText(msg, 32) - 32, 32, 32, DARKGRAY);
}
