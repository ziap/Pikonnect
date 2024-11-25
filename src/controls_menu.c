#include "controls_menu.h"

#include <stddef.h>
#include <raylib.h>

static bool is_pressing_set(const KeyboardKey keys[], size_t keys_len) {
  for (size_t i = 0; i < keys_len; ++i) {
    if (IsKeyPressed(keys[i]) || IsKeyPressedRepeat(keys[i])) return true;
  }
  return false;
}

bool ControlsMenu_down(void) {
  const KeyboardKey keys[] = { KEY_DOWN, KEY_J, KEY_S };
  return is_pressing_set(keys, sizeof(keys) / sizeof(keys[0]));
}

bool ControlsMenu_up(void) {
  const KeyboardKey keys[] = { KEY_UP, KEY_K, KEY_W };
  return is_pressing_set(keys, sizeof(keys) / sizeof(keys[0]));
}

bool ControlsMenu_confirm(void) {
  const KeyboardKey keys[] = { KEY_ENTER, KEY_SPACE };
  return is_pressing_set(keys, sizeof(keys) / sizeof(keys[0]));
}

bool ControlsMenu_left(void) {
  const KeyboardKey keys[] = { KEY_LEFT, KEY_H, KEY_A };
  return is_pressing_set(keys, sizeof(keys) / sizeof(keys[0]));
}

bool ControlsMenu_right(void) {
  const KeyboardKey keys[] = { KEY_RIGHT, KEY_L, KEY_D };
  return is_pressing_set(keys, sizeof(keys) / sizeof(keys[0]));
}

bool ControlsMenu_tab_next(void) {
  if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

bool ControlsMenu_tab_prev(void) {
  if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}
