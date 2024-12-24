#include "pause_menu.hpp"
#include "ui.hpp"

void PauseMenu::update() {
  background.set_width(ui.get_window_width());
  background.set_height(ui.get_window_height());
}

void PauseMenu::draw() {
}