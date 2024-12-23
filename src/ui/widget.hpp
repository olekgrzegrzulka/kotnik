#pragma once
#include <glm/vec2.hpp>
#include "../common.hpp"

class UI;

#define WIDGET_DEF_SETTER_DIRTY(field)         \
  void set_##field(decltype(field) field##_) { \
    if (field == field##_) { return; }         \
    field = field##_;                          \
    dirty = true;                              \
  }

#define WIDGET_DEF_GETTER(field) \
  decltype(field) get_##field() const { return field; }

enum class Anchor {
  TOP_LEFT,
  TOP_CENTER,
  TOP_RIGHT,
  CENTER_LEFT,
  CENTER_CENTER,
  CENTER_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_CENTER,
  BOTTOM_RIGHT,
};

// clang-format off
inline glm::vec2 anchor_to_uv(Anchor anchor) {
  using enum Anchor;
  switch (anchor) {
  case TOP_LEFT:      {return {0.0, 0.0};}
  case TOP_CENTER:    {return {0.5, 0.0};}
  case TOP_RIGHT:     {return {1.0, 0.0};}
  case CENTER_LEFT:   {return {0.0, 0.5};}
  case CENTER_CENTER: {return {0.5, 0.5};}
  case CENTER_RIGHT:  {return {1.0, 0.5};}
  case BOTTOM_LEFT:   {return {0.0, 1.0};}
  case BOTTOM_CENTER: {return {0.5, 1.0};}
  case BOTTOM_RIGHT:  {return {1.0, 1.0};}
  }
}
// clang-format on

class Widget {
protected:
  const UI& ui;
  i32 x = 0;
  i32 y = 0;
  i32 width = 64;
  i32 height = 64;
  Anchor anchor = Anchor::TOP_LEFT;
  Anchor screen_anchor = Anchor::TOP_LEFT;
  bool dirty = true;

private:
  i32 window_width;
  i32 window_height;

public:
  Widget(const UI& ui_) : ui{ui_} {}

  virtual ~Widget() {};

  virtual void update() = 0;

  virtual void draw() = 0;

  // Returns widget's scene position relative to some anchor
  // (i.e. using Anchor::CENTER_CENTER will yield the center position of the widget)
  glm::vec<2, i32> get_position(Anchor relative_to = Anchor::TOP_LEFT) const;

  WIDGET_DEF_SETTER_DIRTY(x)
  WIDGET_DEF_SETTER_DIRTY(y)
  WIDGET_DEF_SETTER_DIRTY(width)
  WIDGET_DEF_SETTER_DIRTY(height)
  WIDGET_DEF_SETTER_DIRTY(anchor)
  WIDGET_DEF_SETTER_DIRTY(screen_anchor)
  WIDGET_DEF_SETTER_DIRTY(window_width);
  WIDGET_DEF_SETTER_DIRTY(window_height);

  WIDGET_DEF_GETTER(x)
  WIDGET_DEF_GETTER(y)
  WIDGET_DEF_GETTER(width)
  WIDGET_DEF_GETTER(height)
  WIDGET_DEF_GETTER(anchor)
  WIDGET_DEF_GETTER(screen_anchor)
};