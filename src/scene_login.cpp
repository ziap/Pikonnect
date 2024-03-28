#include "scene.h"
#include "config.h"

#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void draw_title() {
  const int TITLE_SIZE = 72;
  const char title[] = "Pikachu Game";
  DrawText(title, (SCREEN_WIDTH - MeasureText(title, TITLE_SIZE)) / 2, 150, TITLE_SIZE, DARKBLUE);
}

static void draw_labels(const char** labels, int x, int y) {
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    DrawText(labels[i], x, y, 32, DARKGRAY);
    y += 64;
  }
}

static void draw_textboxes(int x, int y, int max_width, TextBoxData *textboxes) {
  const char *ptr = textboxes[TEXTBOX_USERNAME].input;
  while (MeasureText(ptr, 32) > max_width) ++ptr;
  DrawText(ptr, x, y, 32, BLACK);
  char pwd[16];
  const TextBoxData *pwd_box = textboxes + TEXTBOX_PASSWORD;
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

static bool is_pressing_down() {
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

static bool is_pressing_up() {
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

Scene Scene_login_update(Game *game, float dt) {
  (void)dt;

  LoginMenu *menu = &game->login_menu;

  const char *textbox_names[TEXTBOX_LEN] = {
    "username",
    "password",
  };

  const char *textbox_labels[TEXTBOX_LEN] = {
    "Username: ",
    "Password: ",
  };

  int max_width = MeasureText("username", 32);
  for (int i = 1; i < TEXTBOX_LEN; ++i) {
    int w = MeasureText(textbox_labels[i], 32);
    max_width = w > max_width ? w : max_width;
  }

  const int input_width = MeasureText("***************", 32);
  const int field_width = max_width + input_width;

  if (is_pressing_down()) {
    menu->selected_textbox = (TextBox)((menu->selected_textbox + 1) % TEXTBOX_LEN);
  }

  if (is_pressing_up()) {
    menu->selected_textbox = (TextBox)((menu->selected_textbox + TEXTBOX_LEN - 1) % TEXTBOX_LEN);
  }

  TextBoxData *box = menu->textboxes + menu->selected_textbox;
  for (int k = GetCharPressed(); k; k = GetCharPressed()) {
    if (k >= 0x20 && k < 0x7f) {
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
      box->input[--box->len] = '\0';
      menu->message[0] = '\0';
    }
  }

  if (IsKeyPressed(KEY_ENTER)) {
    bool logged_in = true;
    for (int i = 0; i < TEXTBOX_LEN; ++i) {
      if (menu->textboxes[i].len == 0) {
        snprintf(menu->message, sizeof(menu->message), "Please enter your %s!", textbox_names[i]);
        menu->selected_textbox = (TextBox)i;
        logged_in = false;
        break;
      }
    }
    if (logged_in) {
      User *login_result = UserTable_login(&game->users, menu->textboxes[TEXTBOX_USERNAME].input, menu->textboxes[TEXTBOX_PASSWORD].input);
      if (login_result == nullptr) {
        snprintf(menu->message, sizeof(menu->message), "Wrong password for user %s!", menu->textboxes[TEXTBOX_USERNAME].input);
      } else {
        game->current_user = login_result;
        return SCENE_HOME;
      }
    }
  }

  const int labels_x = (SCREEN_WIDTH - field_width) / 2;
  const int textbox_x = labels_x + max_width;

  draw_title();
  draw_labels(textbox_labels, labels_x, 320);
  draw_textboxes(textbox_x, 320, input_width, menu->textboxes);
  draw_underlines(textbox_x, 320 + 32, input_width, menu->selected_textbox);

  float x0 = (float)(SCREEN_WIDTH - field_width) / 2;
  float x1 = x0 - 16;
  float x2 = x0 - 32;

  float y0 = 320 + 64 * menu->selected_textbox;
  float y1 = y0 + 8;
  float y2 = y0 + 24;
  float y3 = y0 + 16;
  const Color selector_color = menu->message[0] ? RED : DARKBLUE;
  DrawTriangle({x2, y1}, {x2, y2}, {x1, y3}, selector_color);
  DrawRectangle(x0 + max_width, 320 + 32 * (menu->selected_textbox * 2 + 1), input_width, 5, selector_color);

  const char login_msg[] = "Press [ENTER] to log in";
  DrawText(login_msg, (SCREEN_WIDTH - MeasureText(login_msg, 24)) / 2, 320 + 32 * (TEXTBOX_LEN * 2 + 2), 24, GRAY);
  DrawText(menu->message, (SCREEN_WIDTH - MeasureText(menu->message, 32)) / 2, 320 + 32 * (TEXTBOX_LEN * 2 + 5), 32, RED);

  return SCENE_LOGIN;
}
