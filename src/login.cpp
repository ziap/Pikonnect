#include "login.h"
#include <stdlib.h>

void LoginMenu_init(LoginMenu *menu) {
  menu->selected_textbox = 0;
  menu->message[0] = '\0';

  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    menu->textboxes[i].input = (char*)malloc(16);
    menu->textboxes[i].input[0] = '\0';
    menu->textboxes[i].len = 0;
    menu->textboxes[i].cap = 16;
  }
}

void LoginMenu_deinit(LoginMenu *menu) {
  for (int i = 0; i < TEXTBOX_LEN; ++i) {
    free(menu->textboxes[i].input);
  }
}
