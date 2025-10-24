#include "button.hpp"
#include "../input.hpp"
#include "ui.hpp"

void Button::update() {
  bool mouse_on_widget_x = Input::get_mouse_x() >= get_position(Anchor::TOP_LEFT).x && Input::get_mouse_x() < get_position(Anchor::BOTTOM_RIGHT).x;
  bool mouse_on_widget_y = Input::get_mouse_y() >= get_position(Anchor::TOP_LEFT).y && Input::get_mouse_y() < get_position(Anchor::BOTTOM_RIGHT).y;

  bool mouse_hovering = mouse_on_widget_x && mouse_on_widget_y;
  bool lmb_pressed = Input::mouse_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_pressed = Input::mouse_just_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_released = Input::mouse_just_released(Input::Mouse::MOUSE_BUTTON_LEFT);

  if (switch_mode == false) {
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
  } else {
    ButtonState new_state = ButtonState::IDLE;
    if (state == ButtonState::DISABLED) {
      new_state = ButtonState::DISABLED;
    } else if (mouse_hovering && lmb_just_released) {
      if (state == ButtonState::IDLE) {
        new_state = ButtonState::PRESSED;
        pressed();
      } else if (state == ButtonState::PRESSED) {
        new_state = ButtonState::IDLE;
        depressed();
      }

      set_state(new_state);
    }
  }

  Sprite::update();

  if (state == ButtonState::PRESSED && offset_label_on_press) {
    label.set_x(1);
    label.set_y(1);
  } else {
    label.set_x(0);
    label.set_y(0);
  }
  label.set_screen_anchor(Anchor::CENTER_CENTER);
  label.set_anchor(Anchor::CENTER_CENTER);
}

void Button::press() {
  if (state == ButtonState::DISABLED) { return; }

  if (!switch_mode) {
    set_state(ButtonState::PRESSED);
    pressed();
  } else {
    if (state == ButtonState::IDLE) {
      set_state(ButtonState::PRESSED);
      pressed();
    } else if (state == ButtonState::PRESSED) {
      set_state(ButtonState::IDLE);
      depressed();
    }
  }
}

void Button::depress() {
  if (state == ButtonState::DISABLED) { return; }

  if (!switch_mode) {
    // set_state(ButtonState::PRESSED);
    // pressed();
  } else {
    if (state == ButtonState::IDLE) {
      // set_state(ButtonState::PRESSED);
      // pressed();
    } else if (state == ButtonState::PRESSED) {
      set_state(ButtonState::IDLE);
      depressed();
    }
  }
}

void Button::set_texture_idle(std::string id) {
  auto val = ui.get_texture_atlas().get(id);
  if (!val.has_value()) {
    debug_warn("atlas texture not found: " + id);
    return;
  }
  uv_start_idle = val->get().start;
  uv_end_idle = val->get().end;
  texture_width = val->get().width;
  texture_height = val->get().height;
  if (state == ButtonState::IDLE) { dirty = true; } // FIXME check if texture actually changed
}
void Button::set_texture_hovered(std::string id) {
  auto val = ui.get_texture_atlas().get(id);
  if (!val.has_value()) {
    debug_warn("atlas texture not found: " + id);
    return;
  }
  uv_start_hovered = val->get().start;
  uv_end_hovered = val->get().end;
  texture_width = val->get().width;
  texture_height = val->get().height;
  if (state == ButtonState::HOVERED) { dirty = true; } // FIXME check if texture actually changed
}
void Button::set_texture_pressed(std::string id) {
  auto val = ui.get_texture_atlas().get(id);
  if (!val.has_value()) {
    debug_warn("atlas texture not found: " + id);
    return;
  }
  uv_start_pressed = val->get().start;
  uv_end_pressed = val->get().end;
  texture_width = val->get().width;
  texture_height = val->get().height;
  if (state == ButtonState::PRESSED) { dirty = true; } // FIXME check if texture actually changed
}
void Button::set_texture_disabled(std::string id) {
  auto val = ui.get_texture_atlas().get(id);
  if (!val.has_value()) {
    debug_warn("atlas texture not found: " + id);
    return;
  }
  uv_start_disabled = val->get().start;
  uv_end_disabled = val->get().end;
  texture_width = val->get().width;
  texture_height = val->get().height;
  if (state == ButtonState::DISABLED) { dirty = true; } // FIXME check if texture actually changed
}
