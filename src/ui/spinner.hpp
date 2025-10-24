#pragma once
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include "../input.hpp"
#include "button.hpp"
#include "label.hpp"
#include "sprite.hpp"
#include "text_input.hpp"
#include "widget.hpp"

class Spinner : public TextInput {
public:
  Spinner(UI& ui_) : TextInput(ui_), button_label(add_child<Sprite>()), button_increase(add_child<Button>()), button_decrease(add_child<Button>()) {

    button_label.set_texture("spinner_buttons");
    button_label.set_size(32, 32);
    button_label.set_screen_anchor(Anchor::CENTER_RIGHT);
    button_label.set_anchor(Anchor::CENTER_RIGHT);

    button_increase.set_screen_anchor(Anchor::TOP_RIGHT);
    button_increase.set_anchor(Anchor::TOP_RIGHT);
    button_increase.set_size(32, height / 2);
    button_increase.set_visible(false);
    button_increase.on_press([&]() {
      value = std::clamp(value + button_step, min_value, max_value);
      label.set_text(std::to_string(value));
      on_text_changed();
    });

    button_decrease.set_screen_anchor(Anchor::BOTTOM_RIGHT);
    button_decrease.set_anchor(Anchor::BOTTOM_RIGHT);
    button_decrease.set_size(32, height / 2);
    button_decrease.set_visible(false);
    button_decrease.on_press([&]() {
      value = std::clamp(value - button_step, min_value, max_value);
      label.set_text(std::to_string(value));
      on_text_changed();
    });

    label.set_text(std::to_string(value));
  }

  void on_text_changed() override {
    i32 new_value = 0;
    auto str = label.get_text();

    if (str.empty()) {
      label.set_text(str);
      value = 0;
      return;
    }

    if (str.ends_with('-')) {
      if (str.starts_with('-')) {
        str.erase(0, 1);
      } else {
        str = '-' + str;
      }
    }

    try {
      new_value = std::stoi(str);
    } catch (std::invalid_argument) {
      new_value = value;
    } catch (std::out_of_range) {
      if (str.starts_with('-')) {
        new_value = std::numeric_limits<i32>::min();
      } else {
        new_value = std::numeric_limits<i32>::max();
      }
    }

    new_value = std::clamp(new_value, min_value, max_value);
    value = new_value;

    label.set_text(std::to_string(new_value));
  }

  void update() override {
    TextInput::update();

    button_increase.set_size(32, height / 2);
    button_decrease.set_size(32, height / 2);

    bool mouse_on_widget_x = Input::get_mouse_x() >= get_position(Anchor::TOP_LEFT).x && Input::get_mouse_x() < get_position(Anchor::BOTTOM_RIGHT).x;
    bool mouse_on_widget_y = Input::get_mouse_y() >= get_position(Anchor::TOP_LEFT).y && Input::get_mouse_y() < get_position(Anchor::BOTTOM_RIGHT).y;
    auto scroll = Input::get_mouse_scroll();

    if (focused && mouse_on_widget_x && mouse_on_widget_y) {
      if (scroll.y > 0) {
        i32 new_value = std::clamp(value + button_step, min_value, max_value);
        if (value != new_value) {
          if (lambda_value_changed) { lambda_value_changed(); }
          value = new_value;
        }
        label.set_text(std::to_string(value));
        on_text_changed();
      } else if (scroll.y < 0) {
        i32 new_value = std::clamp(value - button_step, min_value, max_value);
        if (value != new_value) {
          if (lambda_value_changed) { lambda_value_changed(); }
          value = new_value;
        }
        label.set_text(std::to_string(value));
        on_text_changed();
      }
    }
  }

protected:
  Sprite& button_label;
  Button& button_increase;
  Button& button_decrease;

  std::function<void()> lambda_value_changed = nullptr;

  i32 value = 0;
  i32 min_value = 0;
  i32 max_value = 100;
  i32 button_step = 1;

public:
  WIDGET_DEF_GETTER(value);
  WIDGET_DEF_GETTER(min_value);
  WIDGET_DEF_GETTER(max_value);
  WIDGET_DEF_GETTER(button_step);

  void set_value(i32 value_) {
    i32 new_value = std::clamp(value_, min_value, max_value);
    if (value != new_value) {
      if (lambda_value_changed) { lambda_value_changed(); }
      value = new_value;
    }
    label.set_text(std::to_string(value));
    on_text_changed();
  }

  void on_value_changed(std::function<void()> lambda_value_changed_) {
    lambda_value_changed = lambda_value_changed_;
  }

  void set_min_value(i32 min_value_) {
    if (min_value_ == max_value) { return; }
    min_value = min_value_;
    if (value < min_value) {
      value = min_value;
      if (lambda_value_changed) { lambda_value_changed(); }
      on_text_changed();
    }
  }

  void set_max_value(i32 max_value_) {
    if (max_value_ == max_value) { return; }
    max_value = max_value_;
    if (value > max_value) {
      value = max_value;
      if (lambda_value_changed) { lambda_value_changed(); }
      on_text_changed();
    }
  }

  WIDGET_DEF_SETTER(button_step);
};
