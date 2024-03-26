#include "login.h"

void LoginMenu_init(LoginMenu *menu) {
  menu->selected_textbox = TEXTBOX_USERNAME;
  menu->message[0] = '\0';

  const int INIT_CAP = 16;
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    menu->textboxes[i].input = new char[INIT_CAP];
    menu->textboxes[i].input[0] = '\0';
    menu->textboxes[i].len = 0;
    menu->textboxes[i].cap = INIT_CAP;
  }
}

void LoginMenu_deinit(LoginMenu *menu) {
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    delete[] menu->textboxes[i].input;
  }
}
