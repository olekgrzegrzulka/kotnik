#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "aabb.hpp"
#include "common.hpp"
#include "glad/glad.h"
#include "shader.hpp"

class CubeIndicatorRenderer {
private:
  GLuint cube_indicator_vao = 0;
  GLuint cube_indicator_vbo = 0;

  Shader shader;

public:
  CubeIndicatorRenderer() : shader{"cube_indicator"} {
    // Create cube indicator VAO
    glGenVertexArrays(1, &cube_indicator_vao);
    glBindVertexArray(cube_indicator_vao);
    glGenBuffers(1, &cube_indicator_vbo);
  }

  ~CubeIndicatorRenderer() {
    glDeleteBuffers(1, &cube_indicator_vbo);
    glDeleteVertexArrays(1, &cube_indicator_vao);
  }

  void draw(WorldPos camera_pos, const glm::mat4& camera_matrix, CubePos cube_indicator_pos, const std::vector<AABB>& aabbs_) {
    float margin = 0.0005f;

    const std::vector<AABB> aabbs_fallback = {AABB{{0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}}};
    const std::vector<AABB>& aabbs = aabbs_.empty() ? aabbs_fallback : aabbs_;

    shader.use();
    shader.set_uniform_mat4("matrix", camera_matrix);
    shader.set_uniform_float("position", (float)(cube_indicator_pos.x - camera_pos.x), (float)(cube_indicator_pos.y - camera_pos.y), (float)(cube_indicator_pos.z - camera_pos.z));

    for (auto& aabb : aabbs) {
      float x_min = aabb.offset.x - aabb.half_extents.x - margin;
      float x_high = aabb.offset.x + aabb.half_extents.x + margin;
      float y_min = aabb.offset.y - aabb.half_extents.y - margin;
      float y_high = aabb.offset.y + aabb.half_extents.y + margin;
      float z_min = aabb.offset.z - aabb.half_extents.z - margin;
      float z_high = aabb.offset.z + aabb.half_extents.z + margin;

      std::vector<glm::vec<3, float>> vertices = {
          {x_min, y_min, z_min},
          {x_min, y_min, z_high},
          {x_high, y_min, z_high},
          {x_high, y_min, z_min},
          {x_min, y_min, z_min},

          {x_min, y_high, z_min},
          {x_min, y_high, z_high},
          {x_high, y_high, z_high},
          {x_high, y_high, z_min},
          {x_min, y_high, z_min},

          {x_min, y_high, z_high},
          {x_min, y_min, z_high},
          {x_high, y_min, z_high},
          {x_high, y_high, z_high},
          {x_high, y_high, z_min},
          {x_high, y_min, z_min},
          {x_min, y_min, z_min},
          {x_min, y_high, z_min},
          {x_high, y_high, z_min},

      };

      glBindBuffer(GL_ARRAY_BUFFER, cube_indicator_vbo);
      glBindVertexArray(cube_indicator_vao);

      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec<3, float>), vertices.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, cube_indicator_vbo);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec<3, float>), (void*)0);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindVertexArray(cube_indicator_vao);
      glLineWidth(2.5f);
      glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
    }
  }
}; // namespace CubeIndicatorRenderer