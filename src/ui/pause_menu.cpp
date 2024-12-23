#include "pause_menu.hpp"
#include "ui.hpp"

void PauseMenu::update() {
  if (!is_visible) { return; }
  background.set_width(ui.get_window_width());
  background.set_height(ui.get_window_height());

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