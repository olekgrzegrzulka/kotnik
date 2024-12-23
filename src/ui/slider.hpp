#pragma once
#include "../common.hpp"
#include "button.hpp"
#include "sprite.hpp"
#include "widget.hpp"

class UI;

class Slider final : public Widget {
protected:
  Sprite track;
  Sprite thumb;
  bool is_dragged = false;
  i32 value = 0;
  i32 min_value = -10;
  i32 max_value = 10;
  std::function<void(i32)> lambda = nullptr;

public:
  Slider(const UI& ui_) : Widget::Widget(ui_), track(ui_), thumb(ui_) {
    set_height(16);

    track.set_uv_start({2.0 / 16.0, 0.0 / 16.0});
    track.set_uv_end({3.0 / 16.0, 1.0 / 16.0});
    track.set_nine_slice_margin(1);
    track.set_height(16);

    thumb.set_width(16);
    thumb.set_height(16);
  }

  void on_value_changed(std::function<void(i32)> lambda_) {
    lambda = lambda_;
  }

  void update() override;

  void draw() override {
    track.draw();
    thumb.draw();
  }

  WIDGET_DEF_SETTER_DIRTY(value)
  WIDGET_DEF_SETTER_DIRTY(min_value)
  WIDGET_DEF_SETTER_DIRTY(max_value)
};