#include "button.hpp"
#include "../input.hpp"

void Button::update() {
  bool mouse_on_widget_x = Input::get_mouse_x() >= get_position(Anchor::TOP_LEFT).x && Input::get_mouse_x() < get_position(Anchor::BOTTOM_RIGHT).x;
  bool mouse_on_widget_y = Input::get_mouse_y() >= get_position(Anchor::TOP_LEFT).y && Input::get_mouse_y() < get_position(Anchor::BOTTOM_RIGHT).y;

  bool mouse_hovering = mouse_on_widget_x && mouse_on_widget_y;
  bool lmb_pressed = Input::mouse_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_pressed = Input::mouse_just_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_released = Input::mouse_just_released(Input::Mouse::MOUSE_BUTTON_LEFT);

  ButtonState new_state = ButtonState::IDLE;
  if (state == ButtonState::DISABLED) {
    new_state = ButtonState::DISABLED;
  } else if (mouse_hovering) {
    if (lmb_just_pressed) {
      new_state = ButtonState::PRESSED;
    } else if (state == ButtonState::PRESSED) {
      if (lmb_just_released) {
        pressed();
      }
      if (lmb_pressed) {
        new_state = ButtonState::PRESSED;
      }
    } else {
      new_state = ButtonState::HOVERED;
    }
  }

  set_state(new_state);

  Sprite::update();

  auto center = get_position(Anchor::CENTER_CENTER);
  label.set_x(center.x);
  label.set_y(center.y);
  label.set_screen_anchor(Anchor::TOP_LEFT);
  label.set_anchor(Anchor::CENTER_CENTER);
}