#ifndef MENU_LOGIN_H
#define MENU_LOGIN_H

struct TextBoxData {
  char *input;
  int len;
  int cap;
};

enum TextBox {
  TEXTBOX_USERNAME,
  TEXTBOX_PASSWORD,
  TEXTBOX_LEN
};

struct LoginMenu {
  TextBox selected_textbox;
  char message[256];
  TextBoxData textboxes[TEXTBOX_LEN];
};

extern void LoginMenu_init(LoginMenu *menu);
extern void LoginMenu_deinit(LoginMenu *menu);


#endif
