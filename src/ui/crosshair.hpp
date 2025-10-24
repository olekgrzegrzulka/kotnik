
#pragma once
#include "sprite.hpp"
#include "ui.hpp"

class Crosshair : public Sprite {
public:
  Crosshair(UI& ui) : Sprite::Sprite(ui) {
    set_width(32);
    set_height(32);
    set_nine_slice_margin(3); // FIXME: nine slice needed to not render surrounding sprites on spritesheet
    set_texture("crosshair");
    set_anchor(Anchor::CENTER_CENTER);
    set_screen_anchor(Anchor::CENTER_CENTER);
  }
};