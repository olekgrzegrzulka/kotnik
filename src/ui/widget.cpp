#include "widget.hpp"
#include <glm/vec2.hpp>
#include "ui.hpp"

glm::vec<2, i32> Widget::get_position(Anchor relative_to) const {
  glm::vec<2, i32> value = anchor_to_uv(screen_anchor) * glm::vec2{ui.get_window_width(), ui.get_window_height()};
  if (parent) {
    value = parent->get_position(screen_anchor);
  }
  value += glm::vec2(x, y) - glm::vec2(width, height) * anchor_to_uv(anchor);
  value += glm::vec<2, float>{width, height} * anchor_to_uv(relative_to);
  return value;
}

void Widget::update() {

#ifdef WIDGET_DRAW_DEBUG_RECT
  if (!is_debug_rect) {
    if (!debug_rect) {
      debug_rect = &add_child<Sprite>();
      debug_rect->is_debug_rect = true;
      debug_rect->ignore_parents_layout = true;
      ((Sprite*)debug_rect)->set_texture("red");
      ((Sprite*)debug_rect)->set_nine_slice_margin(1);
      ((Sprite*)debug_rect)->set_nine_slice_scale(1);
    }

    ((Sprite*)debug_rect)->set_size(width, height);
  }
#endif

  if (dirty) {

    // FIXME: this can cause a 1 frame delay when children are updated BEFORE parent
    for (auto&& c : children) {
      c->mark_dirty();
    }

    dirty = false;
  }

  if (layout.enabled) {
    using enum LayoutDirection;
    using enum Anchor;
    i32 offset = layout.margin;

    i32 child_direction = (layout.direction == LEFT_TO_RIGHT || layout.direction == TOP_TO_BOTTOM) ? 1 : -1;
    Anchor child_anchor;
    switch (layout.positioning) {
    case LayoutPositioning::CENTER: {
      switch (layout.direction) {
      case LEFT_TO_RIGHT: child_anchor = CENTER_LEFT; break;
      case RIGHT_TO_LEFT: child_anchor = CENTER_RIGHT; break;
      case TOP_TO_BOTTOM: child_anchor = TOP_CENTER; break;
      case BOTTOM_TO_TOP: child_anchor = BOTTOM_CENTER; break;
      }
      break;
    }
    case LayoutPositioning::LEFT_OR_TOP: {
      switch (layout.direction) {
      case LEFT_TO_RIGHT: child_anchor = TOP_LEFT; break;
      case RIGHT_TO_LEFT: child_anchor = TOP_RIGHT; break;
      case TOP_TO_BOTTOM: child_anchor = TOP_LEFT; break;
      case BOTTOM_TO_TOP: child_anchor = BOTTOM_LEFT; break;
      }
      break;
    }
    case LayoutPositioning::RIGHT_OR_BOTTOM: {
      switch (layout.direction) {
      case LEFT_TO_RIGHT: child_anchor = BOTTOM_LEFT; break;
      case RIGHT_TO_LEFT: child_anchor = BOTTOM_RIGHT; break;
      case TOP_TO_BOTTOM: child_anchor = TOP_RIGHT; break;
      case BOTTOM_TO_TOP: child_anchor = BOTTOM_RIGHT; break;
      }
      break;
    }
    }

    float total_weight = 0;
    for (auto& c : children) {
      total_weight += c->get_weight();
    }

    for (auto& c : children) {
      if (c->ignore_parents_layout) { continue; }

      c->set_anchor(child_anchor);
      c->set_screen_anchor(child_anchor);

      if (layout.direction == LEFT_TO_RIGHT || layout.direction == RIGHT_TO_LEFT) {
        if (!layout.fill) {
          c->set_x(child_direction * offset);
        } else {
          i32 child_width = (width - 2 * layout.margin - (children.size() - 1) * layout.spacing) / total_weight * c->get_weight();
          c->set_x(child_direction * offset);
          c->set_width(child_width);
        }
        if (layout.expand_children) {
          c->set_height(height - 2 * layout.margin);
        }

        offset += c->get_width() + layout.spacing;
      }

      if (layout.direction == TOP_TO_BOTTOM || layout.direction == BOTTOM_TO_TOP) {
        if (!layout.fill) {
          c->set_y(child_direction * offset);
        } else {
          i32 child_height = (height - 2 * layout.margin - (children.size() - 1) * layout.spacing) / children.size();
          c->set_y(child_direction * offset);
          c->set_height(child_height);
        }

        if (layout.expand_children) {
          c->set_width(width - 2 * layout.margin);
        }

        offset += c->get_height() + layout.spacing;
      }
    }

    offset -= layout.spacing;
    offset += layout.margin;

    if (layout.fit_to_contents) {
      if (layout.direction == LEFT_TO_RIGHT || layout.direction == RIGHT_TO_LEFT) {
        set_width(offset);
      }
      if (layout.direction == TOP_TO_BOTTOM || layout.direction == BOTTOM_TO_TOP) {
        set_height(offset);
      }
    }
  }
}