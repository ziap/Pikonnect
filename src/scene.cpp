#include "scene.h"
#include "config.h"

#include <raylib.h>
#include <stdio.h>

#include "scene_login.h"
#include "scene_home.h"
#include "scene_levels.h"
#include "scene_game.h"
#include "scene_leaderboard.h"

void Scene_draw_header(Game *game, const char *menu_name) {
  DrawRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, LIGHTGRAY);
  DrawText(menu_name, 32, 32, 32, BLACK);

  char msg[1024];
  snprintf(msg, sizeof(msg), "Logged in as: %s", game->current_user->name);
  DrawText(msg, SCREEN_WIDTH - MeasureText(msg, 32) - 32, 32, 32, DARKGRAY);
}

static void Scene_load_noop(Game *game) {
  (void)game;
}

static void Scene_unload_noop(Game *game) {
  (void)game;
}

static void (*const Scene_load_functions[SCENE_LEN])(Game *game) = {
  Scene_login_load,
  Scene_home_load,
  Scene_levels_load,
  Scene_load_noop,
  Scene_load_noop,
};

static Scene (*const Scene_update_functions[SCENE_LEN])(Game *game, float dt) = {
  Scene_login_update,
  Scene_home_update,
  Scene_levels_update,
  Scene_game_update,
  Scene_leaderboard_update
};

static void (*const Scene_unload_functions[SCENE_LEN])(Game *game) = {
  Scene_login_unload,
  Scene_unload_noop,
  Scene_unload_noop,
  Scene_unload_noop,
  Scene_unload_noop,
};

void Scene_init(Scene scene, Game *game) {
  Game_init(game);
  Scene_load_functions[scene](game);
}

void Scene_deinit(Scene scene, Game *game) {
  Scene_load_functions[scene](game);
  Game_deinit(game);
}

Scene Scene_update(Scene scene, Game *game, float dt) {
  Scene next_scene = Scene_update_functions[scene](game, dt);

  if (next_scene != scene) {
    Scene_unload_functions[scene](game);
    Scene_load_functions[next_scene](game);
  }

  return next_scene;
}