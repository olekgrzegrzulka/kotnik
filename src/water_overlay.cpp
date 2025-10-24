#include <initializer_list>
#include <optional>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "common.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#include "water_overlay.hpp"

struct vertex {
  glm::vec<3, float> pos;
};

static GLuint vbo;
static GLuint vao;
static std::vector<vertex> vertices;
static glm::vec4 color;

void water_overlay_init() {
  water_overlay_set_color(std::make_optional<rgba>({0, 30, 205, 140}));

  static constexpr std::initializer_list<vertex> left = {
      vertex{glm::vec3{-1.0, -1.0, +1.0}},
      vertex{glm::vec3{-1.0, -1.0, -1.0}},
      vertex{glm::vec3{-1.0, +1.0, -1.0}},
      vertex{glm::vec3{-1.0, +1.0, -1.0}},
      vertex{glm::vec3{-1.0, +1.0, +1.0}},
      vertex{glm::vec3{-1.0, -1.0, +1.0}},
  };

  static constexpr std::initializer_list<vertex> right = {
      vertex{glm::vec3{+1.0, -1.0, -1.0}},
      vertex{glm::vec3{+1.0, -1.0, +1.0}},
      vertex{glm::vec3{+1.0, +1.0, +1.0}},
      vertex{glm::vec3{+1.0, +1.0, +1.0}},
      vertex{glm::vec3{+1.0, +1.0, -1.0}},
      vertex{glm::vec3{+1.0, -1.0, -1.0}},
  };

  static constexpr std::initializer_list<vertex> front = {
      vertex{glm::vec3{-1.0, -1.0, -1.0}},
      vertex{glm::vec3{+1.0, -1.0, -1.0}},
      vertex{glm::vec3{+1.0, +1.0, -1.0}},
      vertex{glm::vec3{+1.0, +1.0, -1.0}},
      vertex{glm::vec3{-1.0, +1.0, -1.0}},
      vertex{glm::vec3{-1.0, -1.0, -1.0}},
  };

  static constexpr std::initializer_list<vertex> back = {
      vertex{glm::vec3{+1.0, -1.0, +1.0}},
      vertex{glm::vec3{-1.0, -1.0, +1.0}},
      vertex{glm::vec3{-1.0, +1.0, +1.0}},
      vertex{glm::vec3{-1.0, +1.0, +1.0}},
      vertex{glm::vec3{+1.0, +1.0, +1.0}},
      vertex{glm::vec3{+1.0, -1.0, +1.0}},
  };

  static constexpr std::initializer_list<vertex> bottom = {
      vertex{glm::vec3{+1.0, -1.0, +1.0}},
      vertex{glm::vec3{+1.0, -1.0, -1.0}},
      vertex{glm::vec3{-1.0, -1.0, -1.0}},
      vertex{glm::vec3{-1.0, -1.0, -1.0}},
      vertex{glm::vec3{-1.0, -1.0, +1.0}},
      vertex{glm::vec3{+1.0, -1.0, +1.0}},
  };

  static constexpr std::initializer_list<vertex> top = {
      vertex{glm::vec3{-1.0, +1.0, -1.0}},
      vertex{glm::vec3{+1.0, +1.0, -1.0}},
      vertex{glm::vec3{+1.0, +1.0, +1.0}},
      vertex{glm::vec3{+1.0, +1.0, +1.0}},
      vertex{glm::vec3{-1.0, +1.0, +1.0}},
      vertex{glm::vec3{-1.0, +1.0, -1.0}},
  };

  vertices.insert(vertices.cend(), left);
  vertices.insert(vertices.cend(), right);
  vertices.insert(vertices.cend(), front);
  vertices.insert(vertices.cend(), back);
  vertices.insert(vertices.cend(), bottom);
  vertices.insert(vertices.cend(), top);

  // VBO
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
  // VAO
  glGenVertexArrays(1, (GLuint*)&vao);
  glBindVertexArray(vao);
  // Bind vertex position
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // vertex.pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
  // unbind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
};

void water_overlay_draw(const glm::mat4& camera_matrix) {
  ensure(vbo != 0 && vao != 0);

  static Shader sky_shader{"water_overlay"};

  sky_shader.use();
  sky_shader.set_uniform_mat4("camera_matrix", camera_matrix);
  sky_shader.set_uniform_float("color", color.r, color.g, color.b, color.a);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  glBindVertexArray(0);
}

void water_overlay_set_color(std::optional<rgba> color_) {

  if (color_.has_value()) { color = {color_.value().r / 255.0f, color_.value().g / 255.0f, color_.value().b / 255.0f, color_.value().a / 255.0f}; }
}
