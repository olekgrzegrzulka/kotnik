#include "pause_menu.hpp"
#include "ui.hpp"

void PauseMenu::update() {
  if (!is_visible) { return; }
  i32 window_width = ui.get_window_width();
  i32 window_height = ui.get_window_height();

  background.set_width(ui.get_window_width());
  background.set_height(ui.get_window_height());

  background.set_window_width(window_width);
  background.set_window_height(window_height);
  button_resume.set_window_width(window_width);
  button_resume.set_window_height(window_height);
  button_quit.set_window_width(window_width);
  button_quit.set_window_height(window_height);

  background.update();
  button_resume.update();
  button_quit.update();
}

void PauseMenu::draw() {
  if (!is_visible) { return; }
  background.draw();
  button_resume.draw();
  button_quit.draw();
}