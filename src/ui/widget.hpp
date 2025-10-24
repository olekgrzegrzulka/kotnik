#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <glm/vec2.hpp>
#include "../debug.hpp"
#include "../types.hpp"

class UI;

#define WIDGET_DEF_SETTER_DIRTY(field)         \
  void set_##field(decltype(field) field##_) { \
    if (field == field##_) { return; }         \
    field = field##_;                          \
    dirty = true;                              \
  }

#define WIDGET_DEF_SETTER(field)               \
  void set_##field(decltype(field) field##_) { \
    if (field == field##_) { return; }         \
    field = field##_;                          \
  }

#define WIDGET_DEF_GETTER(field) \
  decltype(field) get_##field() const { return field; }

enum class LayoutDirection {
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT,
  TOP_TO_BOTTOM,
  BOTTOM_TO_TOP,
};

enum class LayoutPositioning {
  CENTER,
  LEFT_OR_TOP,
  RIGHT_OR_BOTTOM,
};

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

  LEFT = CENTER_LEFT,
  RIGHT = CENTER_RIGHT,
  TOP = TOP_CENTER,
  BOTTOM = BOTTOM_CENTER,
  CENTER = CENTER_CENTER,
};

inline std::string anchor_to_string(Anchor anchor) {
  using enum Anchor;
  switch (anchor) {
  case TOP_LEFT: return "TOP_LEFT";
  case TOP_CENTER: return "TOP_CENTER";
  case TOP_RIGHT: return "TOP_RIGHT";
  case CENTER_LEFT: return "CENTER_LEFT";
  case CENTER_CENTER: return "CENTER_CENTER";
  case CENTER_RIGHT: return "CENTER_RIGHT";
  case BOTTOM_LEFT: return "BOTTOM_LEFT";
  case BOTTOM_CENTER: return "BOTTOM_CENTER";
  case BOTTOM_RIGHT: return "BOTTOM_RIGHT";
  }
  ensure(false);
  return "";
}

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
  ensure(false);
  return {0.0, 0.0};
}
// clang-format on

class Widget {
protected:
  UI& ui;
  bool process = true;
  i32 x = 0;
  i32 y = 0;
  i32 width = 64;
  i32 height = 64;
  Anchor anchor = Anchor::TOP_LEFT;
  Anchor screen_anchor = Anchor::TOP_LEFT; // parent anchor??
  bool dirty = true;
  bool visible = true;
  bool clip_children = false;
  bool ignore_parents_layout = false;
  bool is_drawn_on_top = false;
  float weight = 1.0f;

  Widget* parent = nullptr;
  std::vector<std::unique_ptr<Widget>> children;
  bool process_children_first = false;

  struct {
    bool enabled = false;
    bool fit_to_contents = false;
    bool expand_children = false;
    bool fill = false;
    i32 margin = 4;
    i32 spacing = 4;
    LayoutDirection direction{};
    LayoutPositioning positioning{};
  } layout;

private:
  i32 window_width = 0;
  i32 window_height = 0;
#ifdef WIDGET_DRAW_DEBUG_RECT
  bool is_debug_rect = false;
  Widget* debug_rect{};
#endif

public:
  Widget(UI& ui_) : ui{ui_} {}

  virtual ~Widget() {};

  virtual void update();

  virtual void draw() {};

  // Returns widget's scene position relative to some anchor
  // (i.e. using Anchor::CENTER_CENTER will yield the center position of the widget)
  glm::vec<2, i32> get_position(Anchor relative_to = Anchor::TOP_LEFT) const;

  WIDGET_DEF_SETTER_DIRTY(process)
  WIDGET_DEF_SETTER_DIRTY(x)
  WIDGET_DEF_SETTER_DIRTY(y)
  WIDGET_DEF_SETTER_DIRTY(width)
  WIDGET_DEF_SETTER_DIRTY(height)
  WIDGET_DEF_SETTER_DIRTY(anchor)
  WIDGET_DEF_SETTER_DIRTY(screen_anchor)
  WIDGET_DEF_SETTER_DIRTY(window_width);
  WIDGET_DEF_SETTER_DIRTY(window_height);
  WIDGET_DEF_SETTER_DIRTY(process_children_first)
  WIDGET_DEF_SETTER_DIRTY(visible)
  WIDGET_DEF_SETTER_DIRTY(clip_children)
  WIDGET_DEF_SETTER_DIRTY(ignore_parents_layout)
  WIDGET_DEF_SETTER_DIRTY(is_drawn_on_top)
  WIDGET_DEF_SETTER_DIRTY(weight)

  WIDGET_DEF_GETTER(process)
  WIDGET_DEF_GETTER(x)
  WIDGET_DEF_GETTER(y)
  WIDGET_DEF_GETTER(width)
  WIDGET_DEF_GETTER(height)
  WIDGET_DEF_GETTER(anchor)
  WIDGET_DEF_GETTER(screen_anchor)
  WIDGET_DEF_GETTER(process_children_first)
  WIDGET_DEF_GETTER(visible)
  WIDGET_DEF_GETTER(clip_children)
  WIDGET_DEF_GETTER(ignore_parents_layout)
  WIDGET_DEF_GETTER(is_drawn_on_top)
  WIDGET_DEF_GETTER(weight)

  auto& get_layout() {
    mark_dirty();
    return layout;
  }

  void set_pos(i32 x_, i32 y_) {
    if (x_ != x || y_ != y) {
      x = x_;
      y = y_;
      dirty = true;
    }
  }

  void set_pos(glm::vec<2, i32> pos) {
    if (pos.x != x || pos.y != y) {
      x = pos.x;
      y = pos.y;
      dirty = true;
    }
  }

  void set_size(i32 w_, i32 h_) {
    if (w_ != width || h_ != height) {
      width = w_;
      height = h_;
      dirty = true;
    }
  }

  void set_size(glm::vec<2, i32> size_) {
    if (width != x || height != y) {
      width = size_.x;
      height = size_.y;
      dirty = true;
    }
  }

  void mark_dirty() {
    dirty = true;
  }

  auto& get_children() {
    return children;
  }

public:
  template <class T, class... Args>
  T& add_child(Args&&... args) {
    static_assert(std::is_base_of_v<Widget, T>);
    children.emplace_back(std::make_unique<T>(ui, std::forward<Args&&...>(args)...));
    T& widget = static_cast<T&>(*children.back().get());
    widget.parent = this;
    return widget;
  }

  void move_child_to_top(const Widget& child) {
    auto it = std::find_if(children.begin(), children.end(), [&](auto&& a) {
      return a.get() == &child;
    });

    if (it != children.end()) {
      auto temp = std::move(*it);
      children.erase(it);
      children.insert(children.begin(), std::move(temp));
    }
  }

  void move_child_to_bottom(const Widget& child) {
    auto it = std::find_if(children.begin(), children.end(), [&](auto&& a) {
      return a.get() == &child;
    });

    if (it != children.end()) {
      auto temp = std::move(*it);
      children.erase(it);
      children.insert(children.end(), std::move(temp));
    }
  }

  // bool remove_child(Widget* w) {
  //   for (size_t i = 0; i < children.size(); i += 1) {
  //     if (children[i].get() == w) {
  //       children.erase(children.begin() + i);
  //       return true;
  //     }
  //   }
  //   return false;
  // }
};