#include "scene.h"

#include <stdio.h>

#include "config.h"
#include "scene_login.h"
#include "scene_home.h"
#include "scene_levels.h"
#include "scene_game.h"
#include "scene_over.h"
#include "scene_leaderboard.h"

// A Finite-state machine based scene management system.
// Scenes are split into four parts:
// - The data (stored in `game.h`)
// - The load function
// - The unload function
// - The update function

// A common functionality between scenes: Draw the header
void Scene_draw_header(Game *game, const char *menu_name) {
  DrawRectangle(0, 0, GetScreenWidth(), HEADER_HEIGHT, { 117, 222, 254, 255 });
  DrawText(menu_name, 32, 32, 32, BLACK);

  char msg[1024];
  snprintf(msg, sizeof(msg), "Logged in as: %s", game->current_user->name);
  DrawText(msg, GetScreenWidth() - MeasureText(msg, 32) - 32, 32, 32, DARKGRAY);
}

// Some scenes do nothing while being unloaded
static void Scene_unload_noop(Game *game) {
  (void)game;
}

// The load, update, and unload functions for each scenes are dynamically
// dispatched using a table of function pointers.

static void (*const Scene_load_functions[SCENE_LEN])(Game *game) = {
  Scene_login_load,
  Scene_home_load,
  Scene_levels_load,
  Scene_game_load,
  Scene_won_load,
  Scene_lost_load,
  Scene_leaderboard_load,
};

static Scene (*const Scene_update_functions[SCENE_LEN])(Game *game, float dt) = {
  Scene_login_update,
  Scene_home_update,
  Scene_levels_update,
  Scene_game_update,
  Scene_won_update,
  Scene_lost_update,
  Scene_leaderboard_update
};

static void (*const Scene_unload_functions[SCENE_LEN])(Game *game) = {
  Scene_login_unload,
  Scene_unload_noop,
  Scene_unload_noop,
  Scene_game_unload,
  Scene_unload_noop,
  Scene_unload_noop,
  Scene_unload_noop,
};

void Scene_init(Scene scene, Game *game) {
  Game_init(game);
  Scene_load_functions[scene](game);
}

void Scene_deinit(Scene scene, Game *game) {
  Scene_unload_functions[scene](game);
  Game_deinit(game);
}

// The update function is responsible for scene switching
Scene Scene_update(Scene scene, Game *game, float dt) {
  Scene next_scene = Scene_update_functions[scene](game, dt);

  if (next_scene != scene) {
    // Scene transition detected
    Scene_unload_functions[scene](game);
    Scene_load_functions[next_scene](game);
  }

  return next_scene;
}
