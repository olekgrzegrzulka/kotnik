#include <glm/gtc/type_ptr.hpp>
#include "common.hpp"
#include "shader.hpp"

namespace CubeIndicatorRenderer {
static GLuint cube_indicator_program = 0;
static GLuint cube_indicator_vao = 0;
static std::vector<glm::vec<3, float>> cube_indicator_vertices;

static void init() {
  // Create cube indicator shader program
  GLuint cube_indicator_vertex = compile_vertex_shader(read_file("shaders/cube_indicator.vert"));
  GLuint cube_indicator_fragment = compile_fragment_shader(read_file("shaders/cube_indicator.frag"));
  cube_indicator_program = glCreateProgram();
  glAttachShader(cube_indicator_program, cube_indicator_vertex);
  glAttachShader(cube_indicator_program, cube_indicator_fragment);
  glLinkProgram(cube_indicator_program);

  // Create cube indicator VAO
  glGenVertexArrays(1, &cube_indicator_vao);
  glBindVertexArray(cube_indicator_vao);

  GLuint cube_indicator_vbo;
  cube_indicator_vertices = {
      {-0.0005f, -0.0005f, -0.0005f},
      {-0.0005f, -0.0005f, +1.0005f},
      {+1.0005f, -0.0005f, +1.0005f},
      {+1.0005f, -0.0005f, -0.0005f},
      {-0.0005f, -0.0005f, -0.0005f},

      {-0.0005f, +1.0005f, -0.0005f},
      {-0.0005f, +1.0005f, +1.0005f},
      {+1.0005f, +1.0005f, +1.0005f},
      {+1.0005f, +1.0005f, -0.0005f},
      {-0.0005f, +1.0005f, -0.0005f},

      {-0.0005f, +1.0005f, +1.0005f},
      {-0.0005f, -0.0005f, +1.0005f},
      {+1.0005f, -0.0005f, +1.0005f},
      {+1.0005f, +1.0005f, +1.0005f},
      {+1.0005f, +1.0005f, -0.0005f},
      {+1.0005f, -0.0005f, -0.0005f},
      {-0.0005f, -0.0005f, -0.0005f},
      {-0.0005f, +1.0005f, -0.0005f},
      {+1.0005f, +1.0005f, -0.0005f},

  };
  glGenBuffers(1, &cube_indicator_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_indicator_vbo);
  glBufferData(GL_ARRAY_BUFFER, cube_indicator_vertices.size() * sizeof(glm::vec<3, float>), cube_indicator_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, cube_indicator_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec<3, float>), (void*)0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
static void draw(WorldPos camera_pos, const glm::mat4& camera_matrix, CubePos cube_indicator_pos) {
  glUseProgram(cube_indicator_program);
  glBindVertexArray(cube_indicator_vao);
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera_matrix));
  glUniform3f(1, (float)(cube_indicator_pos.x - camera_pos.x), (float)(cube_indicator_pos.y - camera_pos.y), (float)(cube_indicator_pos.z - camera_pos.z));
  glLineWidth(2.5f);
  glDrawArrays(GL_LINE_STRIP, 0, cube_indicator_vertices.size());
}
}; // namespace CubeIndicatorRenderer