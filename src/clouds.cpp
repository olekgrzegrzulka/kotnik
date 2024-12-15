#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "clouds.hpp"
#include "common.hpp"
#include "glad/glad.h"
#include "shader.hpp"
#include "texture.hpp"

struct vertex2 {
  glm::vec<3, float> pos;
  glm::vec<2, float> uv;
};

static GLuint vbo;
static GLuint vao;
static std::vector<vertex2> vertices;
static glm::vec3 color;

void clouds_init() {
  clouds_set_color(rgb{255, 255, 255});
  constexpr float a = 1.0;
  vertices = {
      vertex2{glm::vec3{-1.0 * a, +1.0 * a, -1.0 * a}, {0.00, 0.00}},
      vertex2{glm::vec3{+1.0 * a, +1.0 * a, -1.0 * a}, {+1.0, 0.00}},
      vertex2{glm::vec3{+1.0 * a, +1.0 * a, +1.0 * a}, {+1.0, +1.0}},
      vertex2{glm::vec3{+1.0 * a, +1.0 * a, +1.0 * a}, {+1.0, +1.0}},
      vertex2{glm::vec3{-1.0 * a, +1.0 * a, +1.0 * a}, {0.00, +1.0}},
      vertex2{glm::vec3{-1.0 * a, +1.0 * a, -1.0 * a}, {0.00, 0.00}},
  };

  // VBO
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex2), vertices.data(), GL_STATIC_DRAW);
  // VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  // Bind vertex position
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // vertex.pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex2), (void*)offsetof(vertex2, pos));
  // vertex.uv
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2), (void*)offsetof(vertex2, uv));
  // unbind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
};

void clouds_draw(const glm::mat4& camera_matrix) {
  ensure(vbo != 0 && vao != 0);

  static Shader clouds_shader{"clouds"};
  static Texture clouds_texture{"clouds.png"};

  static i32 t = 0;
  t += 1;

  clouds_shader.use();
  clouds_shader.set_uniform_mat4("camera_matrix", camera_matrix);
  clouds_shader.set_uniform_float("color", color.r, color.g, color.b);
  clouds_shader.set_uniform_float("scale", 1.0);
  clouds_shader.set_uniform_i32("t", t);
  clouds_texture.bind(0);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
  glBindVertexArray(0);
}

void clouds_set_color(rgb color_) {
  color = {color_.r / 255.0f, color_.g / 255.0f, color_.b / 255.0f};
}
