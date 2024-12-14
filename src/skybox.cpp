#include "skybox.hpp"
#include <initializer_list>
#include <optional>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "common.hpp"
#include "glad/glad.h"
#include "shader.hpp"

struct vertex {
  glm::vec<3, float> pos;
};

GLuint vbo;
GLuint vao;
std::vector<vertex> vertices;
glm::vec3 color_top;
glm::vec3 color_up;
glm::vec3 color_mid;
glm::vec3 color_down;
glm::vec3 color_bottom;

void skybox_init() {
  skybox_set_color(
      rgb{167, 218, 250},
      rgb{215, 220, 205},
      rgb{245, 221, 171},
      rgb{239, 221, 177},
      rgb{245, 187, 177});

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

void skybox_draw(const glm::mat4& camera_matrix) {
  ensure(vbo != 0 && vao != 0);

  static Shader sky_shader{"sky"};

  sky_shader.use();
  sky_shader.set_uniform_mat4("camera_matrix", camera_matrix);
  sky_shader.set_uniform_float("color_top", color_top.r, color_top.g, color_top.b);
  sky_shader.set_uniform_float("color_up", color_up.r, color_up.g, color_up.b);
  sky_shader.set_uniform_float("color_mid", color_mid.r, color_mid.g, color_mid.b);
  sky_shader.set_uniform_float("color_down", color_down.r, color_down.g, color_down.b);
  sky_shader.set_uniform_float("color_bottom", color_bottom.r, color_bottom.g, color_bottom.b);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  glBindVertexArray(0);
}

void skybox_set_color(std::optional<rgb> top,
                      std::optional<rgb> up,
                      std::optional<rgb> mid,
                      std::optional<rgb> down,
                      std::optional<rgb> bottom) {

  if (top.has_value()) { color_top = {top.value().r / 255.0f, top.value().g / 255.0f, top.value().b / 255.0f}; }
  if (up.has_value()) { color_up = {up.value().r / 255.0f, up.value().g / 255.0f, up.value().b / 255.0f}; }
  if (mid.has_value()) { color_mid = {mid.value().r / 255.0f, mid.value().g / 255.0f, mid.value().b / 255.0f}; }
  if (down.has_value()) { color_down = {down.value().r / 255.0f, down.value().g / 255.0f, down.value().b / 255.0f}; }
  if (bottom.has_value()) { color_bottom = {bottom.value().r / 255.0f, bottom.value().g / 255.0f, bottom.value().b / 255.0f}; }
}
