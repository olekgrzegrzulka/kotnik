#include <glm/gtc/type_ptr.hpp>
#include "common.hpp"
#include "shader.hpp"

class CubeIndicatorRenderer {
private:
  GLuint cube_indicator_vao = 0;
  std::vector<glm::vec<3, float>> cube_indicator_vertices;
  Shader shader;

public:
  CubeIndicatorRenderer() : shader{"cube_indicator"} {
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
  void draw(WorldPos camera_pos, const glm::mat4& camera_matrix, CubePos cube_indicator_pos) {
    shader.use();
    glBindVertexArray(cube_indicator_vao);
    shader.set_uniform_mat4("matrix", camera_matrix);
    shader.set_uniform_float("position", (float)(cube_indicator_pos.x - camera_pos.x), (float)(cube_indicator_pos.y - camera_pos.y), (float)(cube_indicator_pos.z - camera_pos.z));
    glLineWidth(2.5f);
    glDrawArrays(GL_LINE_STRIP, 0, cube_indicator_vertices.size());
  }
}; // namespace CubeIndicatorRenderer