#include "slider.hpp"
#include "../common.hpp"
#include "../input.hpp"
#include "ui.hpp"

void Slider::update() {
  i32 window_width = ui.get_window_width();
  i32 window_height = ui.get_window_height();

  static constexpr i32 x_margin = 10;
  bool mouse_on_widget_x = Input::get_mouse_x() >= get_position(Anchor::TOP_LEFT).x - x_margin &&
                           Input::get_mouse_x() < get_position(Anchor::BOTTOM_RIGHT).x + x_margin;
  bool mouse_on_widget_y = Input::get_mouse_y() >= get_position(Anchor::TOP_LEFT).y &&
                           Input::get_mouse_y() < get_position(Anchor::BOTTOM_RIGHT).y;
  bool mouse_on_widget = mouse_on_widget_x && mouse_on_widget_y;
  bool lmb_pressed = Input::mouse_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_pressed = Input::mouse_just_pressed(Input::Mouse::MOUSE_BUTTON_LEFT);
  bool lmb_just_released = Input::mouse_just_released(Input::Mouse::MOUSE_BUTTON_LEFT);

  if (lmb_just_pressed && mouse_on_widget) {
    is_dragged = true;
  }
  if (lmb_just_released) {
    is_dragged = false;
  }

  i32 old_value = value;
  if (is_dragged && lmb_pressed) {
    i32 x_rel = std::clamp(Input::get_mouse_x() - get_position(Anchor::TOP_LEFT).x, 0, width);
    value = std::round(x_rel / (float)width * (max_value - min_value) + min_value);
  }

  if (old_value != value && lambda) {
    lambda(value);
  }

  track.set_x(x);
  track.set_y(y);
  track.set_width(width);
  track.set_window_width(window_width);
  track.set_window_height(window_height);
  track.set_anchor(anchor);
  track.set_screen_anchor(screen_anchor);

  i32 thumb_x = track.get_position(Anchor::CENTER_CENTER).x - width * 0.5;
  thumb_x += (value - min_value) / (float)(max_value - min_value) * width;
  thumb.set_x(thumb_x);
  thumb.set_y(track.get_position(Anchor::CENTER_CENTER).y);
  thumb.set_window_width(window_width);
  thumb.set_window_height(window_height);
  thumb.set_anchor(Anchor::CENTER_CENTER);
  thumb.set_screen_anchor(Anchor::TOP_LEFT);

  bool mouse_on_thumb_x = Input::get_mouse_x() >= thumb.get_position(Anchor::TOP_LEFT).x &&
                          Input::get_mouse_x() < thumb.get_position(Anchor::BOTTOM_RIGHT).x;
  bool mouse_on_thumb_y = Input::get_mouse_y() >= thumb.get_position(Anchor::TOP_LEFT).y &&
                          Input::get_mouse_y() < thumb.get_position(Anchor::BOTTOM_RIGHT).y;
  bool mouse_on_thumb = mouse_on_thumb_x && mouse_on_thumb_y;
  if (is_dragged) {
    thumb.set_uv_start({2.0 / 16.0, 3.0 / 16.0});
    thumb.set_uv_end({3.0 / 16.0, 4.0 / 16.0});
  } else if (mouse_on_thumb && !lmb_pressed) {
    thumb.set_uv_start({2.0 / 16.0, 2.0 / 16.0});
    thumb.set_uv_end({3.0 / 16.0, 3.0 / 16.0});
  } else {
    thumb.set_uv_start({2.0 / 16.0, 1.0 / 16.0});
    thumb.set_uv_end({3.0 / 16.0, 2.0 / 16.0});
  }

  track.update();
  thumb.update();
}