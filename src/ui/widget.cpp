#include "widget.hpp"
#include <glm/vec2.hpp>
#include "ui.hpp"

glm::vec<2, i32> Widget::get_position(Anchor relative_to) const {
  glm::vec<2, i32> value = anchor_to_uv(screen_anchor) * glm::vec2{ui.get_window_width(), ui.get_window_height()};
  value += glm::vec2(x, y) - glm::vec2(width, height) * anchor_to_uv(anchor);
  value += glm::vec<2, float>{width, height} * anchor_to_uv(relative_to);
  return value;
}