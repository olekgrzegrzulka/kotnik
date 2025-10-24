#include "sprite.hpp"
#include <cstddef>
#include "../glad/glad.h"
#include "ui.hpp"
#include "widget.hpp"

Sprite::Sprite(UI& ui_) : Widget::Widget(ui_) {
}

Sprite::Sprite(UI& ui_, std::string texture_id) : Widget::Widget(ui_) {
  set_texture(texture_id);
}

Sprite::~Sprite() {
}

void Sprite::update() {
  bool dirty_ = dirty;
  Widget::update();

  if (dirty_) {
    update_mesh();
    setup_buffers();
  }
}

void Sprite::draw() {
  auto& sprite_shader = ui.get_sprite_shader();
  // auto& widget_texture = ui.get_widget_texture();

  sprite_shader.use();
  sprite_shader.set_uniform_mat4("matrix", ui.get_matrix());
  // FIXME: don't hardcode texture size
  sprite_shader.set_uniform_float("tex_size", 16, 16);
  ui.get_texture_atlas().bind(0);
  // widget_texture.bind(0);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Sprite::update_mesh() {
  i32 window_width_ = ui.get_window_width();
  i32 window_height_ = ui.get_window_height();

  glm::vec2 size_screen_uv = glm::vec2(width, height) / glm::vec2(window_width_, window_height_);

  glm::vec2 start = glm::vec<2, float>(get_position()) / glm::vec<2, float>{window_width_, window_height_};
  glm::vec2 end = start + size_screen_uv;

  // Translate range from [0, 1] to [-1, 1]
  start = start * glm::vec2(2.0) - glm::vec2(1.0);
  end = end * glm::vec2(2.0) - glm::vec2(1.0);

  vertices = {
      vertex_sprite{{start.x, end.y}, {uv_start.x, uv_end.y}, nine_slice_margin, nine_slice_scale, {width, height}, uv_start, uv_end},
      vertex_sprite{{end.x, end.y}, {uv_end.x, uv_end.y}, nine_slice_margin, nine_slice_scale, {width, height}, uv_start, uv_end},
      vertex_sprite{{start.x, start.y}, {uv_start.x, uv_start.y}, nine_slice_margin, nine_slice_scale, {width, height}, uv_start, uv_end},
      vertex_sprite{{end.x, start.y}, {uv_end.x, uv_start.y}, nine_slice_margin, nine_slice_scale, {width, height}, uv_start, uv_end},
  };
}

void Sprite::setup_buffers() {
  if (vbo != 0) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertex_sprite), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return;
  }

  glGenBuffers(1, &vbo);

  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex_sprite), vertices.data(), GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)offsetof(vertex_sprite, pos));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, uv)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, nine_slice_margin)));
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, nine_slice_scale)));
  glEnableVertexAttribArray(3);

  glVertexAttribIPointer(4, 2, GL_UNSIGNED_INT, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, widget_size)));
  glEnableVertexAttribArray(4);

  glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, uv_start)));
  glEnableVertexAttribArray(5);

  glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_sprite), (void*)(offsetof(vertex_sprite, uv_end)));
  glEnableVertexAttribArray(6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Sprite::set_texture(std::string id) {
  auto val = ui.get_texture_atlas().get(id);
  if (!val.has_value()) {
    debug_warn("atlas texture not found: " + id);
    return;
  }
  uv_start = val->get().start;
  uv_end = val->get().end;
  texture_width = val->get().width;
  texture_height = val->get().height;
  mark_dirty(); // FIXME check if texture actually changed
}