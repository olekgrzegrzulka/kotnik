#pragma once
#include "sprite.hpp"
#include "ui.hpp"
#include "widget.hpp"

class PauseMenu : public Widget {
public:
  PauseMenu(const UI& ui_)
      : Widget::Widget(ui_),
        background(add_child<Sprite>()),
        button_resume(add_child<Button>("Resume")),
        button_quit(add_child<Button>("Quit")) {

    process = false;
    process_children_first = false;

    background.set_uv_start({0.0 / 16.0, 7.0 / 16.0});
    background.set_uv_end({1.0 / 16.0, 8.0 / 16.0});

    button_resume.set_anchor(Anchor::BOTTOM_LEFT);
    button_resume.set_screen_anchor(Anchor::BOTTOM_LEFT);
    button_resume.set_width(200);
    button_resume.set_height(60);
    button_resume.set_x(30);
    button_resume.set_y(-120);

    button_quit.set_anchor(Anchor::BOTTOM_LEFT);
    button_quit.set_screen_anchor(Anchor::BOTTOM_LEFT);
    button_quit.set_width(200);
    button_quit.set_height(60);
    button_quit.set_x(30);
    button_quit.set_y(-30);

    button_resume.on_press([&]() {
      if (process) {
        resume_pressed = true;
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
  Button& button_resume;
  Button& button_quit;
};