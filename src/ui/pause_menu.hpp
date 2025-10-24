#pragma once
#include "button.hpp"
#include "settings_menu.hpp"
#include "sprite.hpp"
#include "widget.hpp"

class UI;

class PauseMenu : public Widget {
public:
  PauseMenu(UI& ui_)
      : Widget::Widget(ui_),
        background(add_child<Sprite>()),
        buttons_container(add_child<Widget>()),
        button_resume(buttons_container.add_child<Button>("Resume")),
        button_settings(buttons_container.add_child<Button>("Settings")),
        button_quit(buttons_container.add_child<Button>("Quit")),
        settings_menu(add_child<SettingsMenu>()) {

    process = false;
    process_children_first = false;

    buttons_container.set_anchor(Anchor::BOTTOM_LEFT);
    buttons_container.set_screen_anchor(Anchor::BOTTOM_LEFT);
    buttons_container.get_layout().enabled = true;
    buttons_container.get_layout().direction = LayoutDirection::TOP_TO_BOTTOM;
    buttons_container.get_layout().fill = true;
    buttons_container.get_layout().expand_children = true;
    buttons_container.get_layout().margin = 10;
    buttons_container.get_layout().spacing = 20;
    buttons_container.set_size(220, 220);

    background.set_texture("pause_menu_bg");
    background.set_nine_slice_margin(3); // FIXME: nine slice needed to not render surrounding sprites on spritesheet

    button_resume.set_width(200);
    button_resume.set_height(60);

    button_quit.set_width(200);
    button_quit.set_height(60);

    button_resume.on_press([&]() {
      if (process) {
        resume_pressed = true;
      }
    });

    button_settings.on_press([&]() {
      if (process) {
        settings_menu.set_process(!settings_menu.get_process());
      }
    });

    button_quit.on_press([&]() {
      if (process) {
        quit_pressed = true;
      }
    });
  }

  void update() override;

  void draw() override;

  bool quit_pressed = false;
  bool resume_pressed = false;

protected:
  Sprite& background;
  Widget& buttons_container;
  Button& button_resume;
  Button& button_settings;
  Button& button_quit;
  SettingsMenu& settings_menu;
};