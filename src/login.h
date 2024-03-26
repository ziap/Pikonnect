#ifndef LOGIN_H
#define LOGIN_H

struct TextBoxData {
  char *input;
  int len;
  int cap;
};

enum TextBoxes {
  TEXTBOX_USERNAME,
  TEXTBOX_PASSWORD,
  TEXTBOX_LEN
};

struct LoginMenu {
  int selected_textbox;
  char message[256];
  TextBoxData textboxes[TEXTBOX_LEN];
};

extern void LoginMenu_init(LoginMenu *menu);
extern void LoginMenu_deinit(LoginMenu *menu);


#endif
