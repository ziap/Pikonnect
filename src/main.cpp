#include <raylib.h>
#include <stdio.h>
#include "scene.h"
#include "config.h"

int main() {
  // Initalize the window
  SetConfigFlags(FLAG_VSYNC_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pikachu game");

  Scene scene = SCENE_LOGIN;
  Game game = Game_create();

  while (!WindowShouldClose()) {
    // Reset the screen
    float dt = GetFrameTime();
    BeginDrawing();
    ClearBackground(WHITE);
  
    // Update the game
    scene = Scene_update_functions[scene](&game, dt);

    // Display the FPS
    char buf[64];
    snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
    DrawText(buf, 20, 20, 20, GREEN);
    EndDrawing();
  }

  // Clean up
  Game_deinit(&game);
  CloseWindow();

  return 0;
}
