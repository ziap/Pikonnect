#include "controls_menu.h"

#include <raylib.h>

bool ControlsMenu_down() {
  const KeyboardKey keys[] = { KEY_DOWN, KEY_J, KEY_S };
  for (KeyboardKey key : keys) {
    if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) return true;
  }

  return false;
}

bool ControlsMenu_up() {
  const KeyboardKey keys[] = { KEY_UP, KEY_K, KEY_W };
  for (KeyboardKey key : keys) {
    if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) return true;
  }

  return false;
}

bool ControlsMenu_confirm() {
  const KeyboardKey keys[] = { KEY_ENTER, KEY_SPACE };
  for (KeyboardKey key : keys) {
    if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) return true;
  }

  return false;
}

bool ControlsMenu_left() {
  const KeyboardKey keys[] = { KEY_LEFT, KEY_H, KEY_A };
  for (KeyboardKey key : keys) {
    if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) return true;
  }

  return false;
}

bool ControlsMenu_right() {
  const KeyboardKey keys[] = { KEY_RIGHT, KEY_L, KEY_D };
  for (KeyboardKey key : keys) {
    if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) return true;
  }

  return false;
}

bool ControlsMenu_tab_next() {
  if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

bool ControlsMenu_tab_prev() {
  if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}
