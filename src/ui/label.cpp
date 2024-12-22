#include <algorithm>
#include <limits>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "../common.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"
#include "font_face.hpp"
#include "label.hpp"
#include "ui.hpp"
#include "widget.hpp"

Label::Label(const UI& ui_, std::u32string text_) : Widget::Widget(ui_) {
  set_text(text_);
}

Label::~Label() {
}

void Label::update() {
  if (dirty) {
    update_mesh();
    setup_buffers();
    dirty = false;
  }
}

void Label::draw() {
  auto& text_shader = ui.get_text_shader();
  auto& font_face = ui.get_font_face();

  text_shader.use();
  text_shader.set_uniform_mat4("matrix", ui.get_matrix());
  text_shader.set_uniform_float("color", text_color.r, text_color.g, text_color.b);
  font_face.bind(0);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

void Label::update_mesh() {
  vertices.clear();
  width = 0;
  height = 0;

  glm::vec<2, i32> max_bearing = {std::numeric_limits<i32>::min(), std::numeric_limits<i32>::min()};

  // // calculate text dimensions
  i32 text_length = 0;
  for (auto c : text) {
    if (text_length++ > max_text_length) { break; }

    auto* glyph = ui.get_font_face().find_glyph(c);
    if (!glyph) { continue; }

    width += glyph->advance.x / 64.0;
    height = std::max(height, glyph->size.y);

    max_bearing.x = std::max(max_bearing.x, glyph->bearing.x);
    max_bearing.y = std::max(max_bearing.y, glyph->bearing.y);
  }

  // calculate initial pen position
  glm::vec2 pen = {x, y};
  pen.y += max_bearing.y;
  pen += anchor_to_uv(screen_anchor) * glm::vec2{window_width, window_height};
  pen.x -= width * anchor_to_uv(anchor).x;
  pen.y -= height * anchor_to_uv(anchor).y;

  for (auto c : text) {
    auto* glyph = ui.get_font_face().find_glyph(c);
    if (!glyph) { continue; }

    auto size_screen_uv = glm::vec2(glyph->size.x, glyph->size.y) / glm::vec2(window_width, window_height);
    auto position_screen_uv = glm::vec2(pen.x + glyph->bearing.x, pen.y - glyph->bearing.y) / glm::vec2{window_width, window_height};

    auto start = position_screen_uv;
    auto end = start + size_screen_uv;

    // Translate range from [0, 1] to [-1, 1]
    start = start * glm::vec2(2.0) - glm::vec2(1.0);
    end = end * glm::vec2(2.0) - glm::vec2(1.0);

    auto uv_start = glyph->uv_start;
    auto uv_end = glyph->uv_end;

    vertices.emplace_back(vertex2{{start.x, end.y}, {uv_start.x, uv_end.y}});
    vertices.emplace_back(vertex2{{end.x, end.y}, {uv_end.x, uv_end.y}});
    vertices.emplace_back(vertex2{{start.x, start.y}, {uv_start.x, uv_start.y}});

    vertices.emplace_back(vertex2{{end.x, end.y}, {uv_end.x, uv_end.y}});
    vertices.emplace_back(vertex2{{end.x, start.y}, {uv_end.x, uv_start.y}});
    vertices.emplace_back(vertex2{{start.x, start.y}, {uv_start.x, uv_start.y}});

    pen.x += glyph->advance.x / 64.0;
    pen.y += glyph->advance.y / 64.0;
  }
}

void Label::setup_buffers() {
  glGenBuffers(1, &vbo);

  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex2), vertices.data(), GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2), (void*)offsetof(vertex2, pos));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2), (void*)(offsetof(vertex2, uv)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}