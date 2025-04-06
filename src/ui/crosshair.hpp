
#pragma once
#include "sprite.hpp"
#include "ui.hpp"

class Crosshair : public Sprite {
public:
  Crosshair(UI& ui) : Sprite::Sprite(ui) {
    set_width(32);
    set_height(32);
    set_uv_start({0.0 / 16.0, 6.0 / 16.0});
    set_uv_end({1.0 / 16.0, 7.0 / 16.0});
    set_anchor(Anchor::CENTER_CENTER);
    set_screen_anchor(Anchor::CENTER_CENTER);
  }
};