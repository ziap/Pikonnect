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
  DrawRectangle(0, 0, GetScreenWidth(), HEADER_HEIGHT, (Color) { 142, 228, 255, 255 });
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
  [SCENE_LOGIN] = Scene_login_load,
  [SCENE_HOME] = Scene_home_load,
  [SCENE_LEVELS] = Scene_levels_load,
  [SCENE_GAME] = Scene_game_load,
  [SCENE_WON] = Scene_won_load,
  [SCENE_LOST] = Scene_lost_load,
  [SCENE_LEADERBOARD] = Scene_leaderboard_load,
};

static Scene (*const Scene_update_functions[SCENE_LEN])(Game *game, float dt) = {
  [SCENE_LOGIN] = Scene_login_update,
  [SCENE_HOME] = Scene_home_update,
  [SCENE_LEVELS] = Scene_levels_update,
  [SCENE_GAME] = Scene_game_update,
  [SCENE_WON] = Scene_won_update,
  [SCENE_LOST] = Scene_lost_update,
  [SCENE_LEADERBOARD] = Scene_leaderboard_update
};

static void (*const Scene_unload_functions[SCENE_LEN])(Game *game) = {
  [SCENE_LOGIN] = Scene_login_unload,
  [SCENE_HOME] = Scene_unload_noop,
  [SCENE_LEVELS] = Scene_unload_noop,
  [SCENE_GAME] = Scene_game_unload,
  [SCENE_WON] = Scene_unload_noop,
  [SCENE_LOST] = Scene_unload_noop,
  [SCENE_LEADERBOARD] = Scene_unload_noop,
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
