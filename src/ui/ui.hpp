#pragma once
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "../common.hpp"
#include "../shader.hpp"
#include "../texture.hpp"
#include "button.hpp"
#include "font_face.hpp"
#include "freetype/freetype.h"
#include "label.hpp"
#include "slider.hpp"
#include "sprite.hpp"
#include "widget.hpp"

class UI {
public:
  UI(i32 window_width_, i32 window_height_)
      : widget_texture{{"gui.png"}}, sprite_shader{"sprite"}, text_shader{"text"} {

    window_width = window_width_;
    window_height = window_height_;

    if (FT_Init_FreeType(&freetype_lib)) {
      debug_error("failed to initialize freetype");
      return;
    }

    matrix = glm::mat4{1.0};
    matrix = glm::scale(matrix, glm::vec3(1.0f, -1.0f, 1.0f));

    font_face = FontFace(freetype_lib, "./assets/Roboto/Roboto-Regular.ttf", 18);
  }

  template <class T, class... Args>
  T& add_widget(Args&&... args) {
    widgets.emplace_back(std::make_unique<T>(*this, std::forward<Args&&...>(args)...));
    T& widget = static_cast<T&>(*widgets.back().get());
    return widget;
  }

  void update(i32 window_width_, i32 window_height_) {
    window_width = window_width_;
    window_height = window_height_;

    for (auto&& widget : widgets) {
      update_widget_recursive(widget);
    }
  }

  void draw() {
    glDisable(GL_DEPTH_TEST);
    for (auto&& widget : widgets) {
      draw_widget_recursive(widget);
    }
  }

  i32 get_window_width() const { return window_width; }
  i32 get_window_height() const { return window_height; }
  const glm::mat4& get_matrix() const { return matrix; }
  const Texture& get_widget_texture() const { return widget_texture; }
  const FontFace& get_font_face() const { return font_face; }
  const Shader& get_sprite_shader() const { return sprite_shader; }
  const Shader& get_text_shader() const { return text_shader; }

private:
  void update_widget_recursive(std::unique_ptr<Widget>& widget) {
    if (!widget->get_process()) { return; }

    widget->set_window_width(window_width);
    widget->set_window_height(window_height);

    if (!widget->get_process_children_first()) { widget->update(); }

    for (auto&& child : widget->get_children()) {
      child->set_window_width(window_width);
      child->set_window_height(window_height);
      update_widget_recursive(child);
    }

    if (widget->get_process_children_first()) { widget->update(); }
  }

  void draw_widget_recursive(std::unique_ptr<Widget>& widget) {
    if (!widget->get_process()) { return; }

    if (!widget->get_process_children_first()) { widget->draw(); }

    for (auto&& child : widget->get_children()) {
      draw_widget_recursive(child);
    }

    if (widget->get_process_children_first()) { widget->draw(); }
  }

protected:
  glm::mat4 matrix;
  std::vector<std::unique_ptr<Widget>> widgets;
  Texture widget_texture;
  FT_Library freetype_lib;
  Shader sprite_shader;
  FontFace font_face;
  Shader text_shader;
  i32 window_width = 0;
  i32 window_height = 0;
};