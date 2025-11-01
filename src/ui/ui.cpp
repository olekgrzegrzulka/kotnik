#include <memory>
#include <vector>
#include <freetype/freetype.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "../glad/glad.h"
#include "../shader.hpp"
#include "../texture.hpp"
#include "../types.hpp"
#include "font_face.hpp"
#include "label.hpp"
#include "sprite.hpp"
#include "ui.hpp"
#include "widget.hpp"

UI::UI(i32 window_width_, i32 window_height_)
    : sprite_shader{"sprite"}, text_shader{"text"} {
  window_width = window_width_;
  window_height = window_height_;
  if (FT_Init_FreeType(&freetype_lib)) {
    debug_error("failed to initialize freetype");
    return;
  }
  matrix = glm::mat4{1.0};
  matrix = glm::scale(matrix, glm::vec3(1.0f, -1.0f, 1.0f));

  font_face = FontFace(freetype_lib, "./assets/Roboto/Roboto-Regular.ttf", 15);

  texture_atlas.add_texture("red", "./assets/ui/red.png");
  texture_atlas.add_texture("crosshair", "./assets/ui/crosshair.png");
  texture_atlas.add_texture("button_disabled", "./assets/ui/button_disabled.png");
  texture_atlas.add_texture("button_hovered", "./assets/ui/button_hovered.png");
  texture_atlas.add_texture("pause_menu_bg", "./assets/ui/pause_menu_bg.png");
  texture_atlas.add_texture("button_idle", "./assets/ui/button_idle.png");
  texture_atlas.add_texture("button_pressed", "./assets/ui/button_pressed.png");
  texture_atlas.add_texture("tab_disabled", "./assets/ui/tab_disabled.png");
  texture_atlas.add_texture("tab_hovered", "./assets/ui/tab_hovered.png");
  texture_atlas.add_texture("tab_idle", "./assets/ui/tab_idle.png");
  texture_atlas.add_texture("tab_pressed", "./assets/ui/tab_pressed.png");
  texture_atlas.add_texture("combo_box_button_expand", "./assets/ui/combo_box_button_expand.png");
  texture_atlas.add_texture("combo_box_button_contract", "./assets/ui/combo_box_button_contract.png");
  texture_atlas.add_texture("combo_box", "./assets/ui/combo_box.png");
  texture_atlas.add_texture("panel_rectangular_highlighted", "./assets/ui/panel_rectangular_highlighted.png");
  texture_atlas.add_texture("panel_rectangular", "./assets/ui/panel_rectangular.png");
  texture_atlas.add_texture("panel_rounded", "./assets/ui/panel_rounded.png");
  texture_atlas.add_texture("slider_thumb_hovered", "./assets/ui/slider_thumb_hovered.png");
  texture_atlas.add_texture("slider_thumb_idle", "./assets/ui/slider_thumb_idle.png");
  texture_atlas.add_texture("slider_thumb_pressed", "./assets/ui/slider_thumb_pressed.png");
  texture_atlas.add_texture("slider_track", "./assets/ui/slider_track.png");
  texture_atlas.add_texture("spinner_buttons", "./assets/ui/spinner_buttons.png");
  texture_atlas.add_texture("text_input_caret", "./assets/ui/text_input_caret.png");
  texture_atlas.add_texture("text_input_focused", "./assets/ui/text_input_focused.png");
  texture_atlas.add_texture("text_input_idle", "./assets/ui/text_input_idle.png");
}

UI::~UI() {
  FT_Done_FreeType(freetype_lib);
}

void UI::update(i32 window_width_, i32 window_height_) {
  window_width = window_width_;
  window_height = window_height_;

  for (auto&& widget : widgets) {
    update_widget_recursive(widget);
  }
}

void UI::draw() {
  glDisable(GL_DEPTH_TEST);

  std::vector<Widget*> to_be_drawn_later;

  for (auto&& widget : widgets) {
    draw_widget_recursive(widget.get(), &to_be_drawn_later);
  }

  for (auto* widget : to_be_drawn_later) {
    draw_widget_recursive(widget, nullptr);
  }
}

void UI::update_widget_recursive(std::unique_ptr<Widget>& widget) {
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

void UI::draw_widget_recursive(Widget* widget, std::vector<Widget*>* to_be_drawn_later) {
  if (!widget->get_process()) { return; }

  if (to_be_drawn_later && widget->get_is_drawn_on_top()) {
    to_be_drawn_later->emplace_back(widget);
    return;
  }

  // FIXME: children can override their parent's clip rectangle
  if (widget->get_clip_children()) {
    glEnable(GL_SCISSOR_TEST);

    glScissor(widget->get_position(Anchor::TOP_LEFT).x,
              window_height - widget->get_position(Anchor::BOTTOM_RIGHT).y,
              widget->get_width(),
              widget->get_height());
  }

  if (widget->get_visible() && !widget->get_process_children_first()) { widget->draw(); }

  for (auto&& child : widget->get_children()) {
    draw_widget_recursive(child.get(), to_be_drawn_later);
  }

  if (widget->get_visible() && widget->get_process_children_first()) { widget->draw(); }

  if (widget->get_clip_children()) {
    glDisable(GL_SCISSOR_TEST);
  }
}
