#include "config.h"
#include "scene.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>

Scene Scene_login_update(Game *game, float dt) {
  (void)dt;

  LoginMenu *menu = &game->login_menu;

  const char textbox_names[TEXTBOX_LEN][16] = {
    "username",
    "password",
  };
  const char textbox_labels[TEXTBOX_LEN][16] = {
    "Username: ",
    "Password: ",
  };

  const int FIELD_SIZE = 32;

  int max_width = MeasureText("username", FIELD_SIZE);
  for (int i = 1; i < TEXTBOX_LEN; ++i) {
    int w = MeasureText(textbox_labels[i], FIELD_SIZE);
    max_width = w > max_width ? w : max_width;
  }

  char underline[16];
  memset(underline, '_', 15);
  underline[15] = 0;
  const int INPUT_WIDTH = MeasureText(underline, FIELD_SIZE);
  const int FIELD_WIDTH = max_width + INPUT_WIDTH;

  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_N) || IsKeyPressedRepeat(KEY_N)) {
      menu->selected_textbox = (menu->selected_textbox + 1) % TEXTBOX_LEN;
    }

    if (IsKeyPressed(KEY_P) || IsKeyPressedRepeat(KEY_P)) {
      menu->selected_textbox = (menu->selected_textbox + TEXTBOX_LEN - 1) % TEXTBOX_LEN;
    }

    if (IsKeyPressed(KEY_L) || IsKeyPressedRepeat(KEY_L)) {
      menu->textboxes[menu->selected_textbox].len = 0;
      menu->textboxes[menu->selected_textbox].input[0] = '\0';
    }
  }

  if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) {
    menu->selected_textbox = (menu->selected_textbox + 1) % TEXTBOX_LEN;
  }

  if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
    menu->selected_textbox = (menu->selected_textbox + TEXTBOX_LEN - 1) % TEXTBOX_LEN;
  }

  TextBoxData *box = menu->textboxes + menu->selected_textbox;
  if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
    if (box->len) {
      box->input[--box->len] = '\0';
      menu->message[0] = '\0';
    }
  }

  for (int k = GetCharPressed(); k; k = GetCharPressed()) {
    if (k >= 0x20 && k < 0x7f) {
      if (box->len + 1 == box->cap) {
        box->cap *= 2;
        char *new_input = new char[box->cap];
        memcpy(new_input, box->input, box->len + 1);

        delete[] box->input;
        box->input = new_input;
      }
      box->input[box->len++] = k;
      box->input[box->len] = '\0';
      menu->message[0] = '\0';
    }
  }

  if (IsKeyPressed(KEY_ENTER)) {
    bool logged_in = true;
    for (int i = 0; i < TEXTBOX_LEN; ++i) {
      if (menu->textboxes[i].len == 0) {
        snprintf(menu->message, sizeof(menu->message), "Please enter your %s!", textbox_names[i]);
        menu->selected_textbox = i;
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
        return SCENE_MAIN;
      }
    }
  }

  float x0 = (float)(SCREEN_WIDTH - FIELD_WIDTH) / 2;
  float x1 = x0 - FIELD_SIZE * 0.5f;
  float x2 = x0 - FIELD_SIZE;

  float y0 = 320 + FIELD_SIZE * 2 * menu->selected_textbox;
  float y1 = y0 + FIELD_SIZE * 0.25f;
  float y2 = y0 + FIELD_SIZE * 0.75f;
  float y3 = y0 + (float)FIELD_SIZE * 0.5f;

  const int TITLE_SIZE = 72;
  const char title[] = "Pikachu Game";
  DrawText(title, (SCREEN_WIDTH - MeasureText(title, TITLE_SIZE)) / 2, 150, TITLE_SIZE, DARKBLUE);
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    DrawText(textbox_labels[i], x0, 320 + FIELD_SIZE * i * 2, FIELD_SIZE, DARKGRAY);
  }

  const char *ptr = menu->textboxes[TEXTBOX_USERNAME].input;
  while (MeasureText(ptr, FIELD_SIZE) > INPUT_WIDTH) ++ptr;
  DrawText(ptr, x0 + max_width, 320, FIELD_SIZE, BLACK);
  char pwd[16];
  const TextBoxData *pwd_box = menu->textboxes + TEXTBOX_PASSWORD;
  const int pwd_len = pwd_box->len < 15 ? pwd_box->len : 15;
  memset(pwd, '*', pwd_len);
  pwd[pwd_len] = 0;
  DrawText(pwd, x0 + max_width, 320 + FIELD_SIZE * 2, FIELD_SIZE, BLACK);

  for (int i = 0; i < menu->selected_textbox; ++i) {
    DrawRectangle(x0 + max_width, 320 + FIELD_SIZE * (i * 2 + 1), INPUT_WIDTH, 5, GRAY);
  }

  for (int i = menu->selected_textbox + 1; i < TEXTBOX_LEN; ++i) {
    DrawRectangle(x0 + max_width, 320 + FIELD_SIZE * (i * 2 + 1), INPUT_WIDTH, 5, GRAY);
  }

  const Color selector_color = menu->message[0] ? RED : DARKBLUE;
  DrawTriangle({x2, y1}, {x2, y2}, {x1, y3}, selector_color);
  DrawRectangle(x0 + max_width, 320 + FIELD_SIZE * (menu->selected_textbox * 2 + 1), INPUT_WIDTH, 5, selector_color);

  const char login_msg[] = "Press [ENTER] to log in";
  DrawText(login_msg, (SCREEN_WIDTH - MeasureText(login_msg, 24)) / 2, 320 + FIELD_SIZE * (TEXTBOX_LEN * 2 + 2), 24, GRAY);
  DrawText(menu->message, (SCREEN_WIDTH - MeasureText(menu->message, FIELD_SIZE)) / 2, 320 + FIELD_SIZE * (TEXTBOX_LEN * 2 + 5), FIELD_SIZE, RED);

  return SCENE_LOGIN;
}
