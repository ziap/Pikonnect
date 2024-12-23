#include "scene_login.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const Color accent_color = { 0, 135, 204, 255 };

static void render_title(void) {
  const char title[] = "PIKONNECT";
  DrawText(title, (GetScreenWidth() - MeasureText(title, 72)) / 2, 150, 72, accent_color);
}

static void render_labels(const char** labels, int x, int y) {
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    DrawText(labels[i], x, y, 32, DARKGRAY);
    y += 64;
  }
}

static void render_textboxes(int x, int y, int max_width, LoginTextBoxData *textboxes) {
  // Get the last characters that fit inside the text box
  const char *ptr = textboxes[TEXTBOX_USERNAME].input;
  while (MeasureText(ptr, 32) > max_width) ++ptr;
  DrawText(ptr, x, y, 32, BLACK);

  // Draw the asterisks
  char pwd[16];
  const LoginTextBoxData *pwd_box = textboxes + TEXTBOX_PASSWORD;

  // Limit the character count to 15
  const int pwd_len = pwd_box->len < 15 ? pwd_box->len : 15;
  memset(pwd, '*', pwd_len);
  pwd[pwd_len] = 0;
  DrawText(pwd, x, y + 32 * 2, 32, BLACK);
}

static void draw_underlines(int x, int y, int width, int selected)  {
  for (int i = 0; i < selected; ++i) {
    DrawRectangle(x, y, width, 5, GRAY);
    y += 32 * 2;
  }

  y += 32 * 2;
  for (int i = selected + 1; i < TEXTBOX_LEN; ++i) {
    DrawRectangle(x, y, width, 5, GRAY);
  }
}

static bool is_pressing_down(void) {
  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_N) || IsKeyPressedRepeat(KEY_N)) {
      return true;
    }
  }

  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) return true;

  if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

static bool is_pressing_up(void) {
  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_P) || IsKeyPressedRepeat(KEY_P)) {
      return true;
    }
  }

  if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) return true;

  if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

static const char *textbox_names[TEXTBOX_LEN] = {
  [TEXTBOX_USERNAME] = "username",
  [TEXTBOX_PASSWORD] = "password",
};

static const char *textbox_labels[TEXTBOX_LEN] = {
  [TEXTBOX_USERNAME] = "Username: ",
  [TEXTBOX_PASSWORD] = "Password: ",
};

void Scene_login_load(Game *game) {
  LoginMenu *menu = &game->menu.login;
  menu->selected_textbox = TEXTBOX_USERNAME;
  menu->message[0] = '\0';

  // Textboxes are implemented using a dynamic-array based stack, but instead of
  // writing a separate data structure the implementation of the stack is
  // inlined here
  const int INIT_CAP = 16;
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    menu->textboxes[i].input = (char*)malloc(INIT_CAP);
    menu->textboxes[i].input[0] = '\0';
    menu->textboxes[i].len = 0;
    menu->textboxes[i].cap = INIT_CAP;
  }

  menu->max_width = MeasureText(textbox_labels[0], 32);
  for (int i = 1; i < TEXTBOX_LEN; ++i) {
    int w = MeasureText(textbox_labels[i], 32);
    if (w > menu->max_width) menu->max_width = w;
  }

  // Set the length of the textbox to the length of 15 asterisks to match the
  // maximum asterisks count
  menu->input_width = MeasureText("***************", 32);
}

void Scene_login_unload(Game *game) {
  LoginMenu *menu = &game->menu.login;
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    free(menu->textboxes[i].input);
  }
}

Scene Scene_login_update(Game *game, float dt) {
  (void)dt;

  LoginMenu *menu = &game->menu.login;

  const int field_width = menu->max_width + menu->input_width;

  if (is_pressing_down()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->selected_textbox = (LoginTextBox)((menu->selected_textbox + 1) % TEXTBOX_LEN);
  }

  if (is_pressing_up()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->selected_textbox = (LoginTextBox)((menu->selected_textbox + TEXTBOX_LEN - 1) % TEXTBOX_LEN);
  }

  LoginTextBoxData *box = menu->textboxes + menu->selected_textbox;
  for (int k = GetCharPressed(); k; k = GetCharPressed()) {
    // Push a character to the textbox stack
    if (k >= 0x20 && k < 0x7f) {
      PlaySound(game->sounds[SOUND_CLICK]);
      if (box->len + 1 == box->cap) {
        box->cap *= 2;
        box->input = (char*)realloc(box->input, box->cap);
      }
      box->input[box->len++] = k;
      box->input[box->len] = '\0';
      menu->message[0] = '\0';
    }
  }

  if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
    if (box->len) {
      PlaySound(game->sounds[SOUND_CLICK]);
      // Pop a character from the textbox stack
      box->input[--box->len] = '\0';
      menu->message[0] = '\0';
    }
  }

  if (IsKeyPressed(KEY_ENTER)) {
    bool logged_in = true;
    for (int i = 0; i < TEXTBOX_LEN; ++i) {
      if (menu->textboxes[i].len == 0) {
        snprintf(menu->message, sizeof(menu->message), "Please enter your %s!", textbox_names[i]);
        menu->selected_textbox = (LoginTextBox)i;
        logged_in = false;
        break;
      }
    }
    if (logged_in) {
      User *login_result = UserTable_login(&game->users, menu->textboxes[TEXTBOX_USERNAME].input, menu->textboxes[TEXTBOX_PASSWORD].input);
      if (login_result == NULL) {
        // Failed to log in
        // Currently the only reason is that the password is incorrect
        snprintf(menu->message, sizeof(menu->message), "Wrong password for user %s!", menu->textboxes[TEXTBOX_USERNAME].input);
      } else {
        PlaySound(game->sounds[SOUND_SELECT]);
        game->current_user = login_result;
        return SCENE_HOME;
      }
    }
  }

  const int labels_x = (GetScreenWidth() - field_width) / 2;
  const int textbox_x = labels_x + menu->max_width;

  render_title();
  render_labels(textbox_labels, labels_x, 320);
  render_textboxes(textbox_x, 320, menu->input_width, menu->textboxes);
  draw_underlines(textbox_x, 320 + 32, menu->input_width, menu->selected_textbox);

  float x0 = (float)(GetScreenWidth() - field_width) / 2;
  float x1 = x0 - 16;
  float x2 = x0 - 32;

  float y0 = 320 + 64 * menu->selected_textbox;
  float y1 = y0 + 8;
  float y2 = y0 + 24;
  float y3 = y0 + 16;
  const Color selector_color = menu->message[0] ? RED : accent_color;
  DrawTriangle((Vector2) {x2, y1}, (Vector2) {x2, y2}, (Vector2) {x1, y3}, selector_color);
  DrawRectangle(x0 + menu->max_width, 320 + 32 * (menu->selected_textbox * 2 + 1), menu->input_width, 5, selector_color);

  const char login_msg[] = "Press [ENTER] to log in";
  DrawText(login_msg, (GetScreenWidth() - MeasureText(login_msg, 24)) / 2, 320 + 32 * (TEXTBOX_LEN * 2 + 2), 24, GRAY);
  DrawText(menu->message, (GetScreenWidth() - MeasureText(menu->message, 32)) / 2, 320 + 32 * (TEXTBOX_LEN * 2 + 5), 32, RED);

  return SCENE_LOGIN;
}
