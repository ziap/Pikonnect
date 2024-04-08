#include <stdio.h>

#include "scene.h"

int main() {
  // Initalize the window
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(1024, 768, "Pikachu game");
  InitAudioDevice();

  Scene scene = SCENE_LOGIN;
  Game game;

  Scene_init(scene, &game);

  while (!WindowShouldClose()) {
    // Reset the screen
    float dt = GetFrameTime();
    BeginDrawing();
    ClearBackground(WHITE);
  
    // Update the game
    scene = Scene_update(scene, &game, dt);

    // Display the FPS
    char buf[64];
    snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
    DrawText(buf, 20, GetScreenHeight() - 40, 20, GREEN);
    EndDrawing();
  }

  // Clean up
  Scene_deinit(scene, &game);
  CloseWindow();

  return 0;
}
