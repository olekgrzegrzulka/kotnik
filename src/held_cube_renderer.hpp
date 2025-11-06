#include <chrono>
#include <vector>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/trigonometric.hpp>
#include "common.hpp"
#include "config.hpp"
#include "cubes.hpp"
#include "glad/glad.h"
#include "random.hpp"
#include "shader.hpp"
#include "texture.hpp"

class HeldCubeRenderer {
private:
  Shader& shader;
  Texture& atlas_texture;

  std::optional<CubeId> cube_id;

  std::vector<CompactVertex> vertices;
  GLuint vao = 0;
  GLuint vbo = 0;

public:
  HeldCubeRenderer(Shader& shader_, Texture& atlas_texture_)
      : shader{shader_}, atlas_texture{atlas_texture_} {
  }

  void draw(i32 window_width, i32 window_height, CubeId new_cube_id) {
    if (!cube_id.has_value() || cube_id.value() != new_cube_id) {
      cube_id = new_cube_id;
      update_mesh(cube_id.value());
    }

    glm::vec2 pixel_size = {1.0f / window_width, 1.0f / window_height}; // FIXME it's not actual pixel size
    static constexpr glm::vec3 light = {0.41f, 0.82f, 0.41f};
    auto now = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    float r = static_cast<float>((t / 32) % 360);

    glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

    glm::mat4 model{1.0f};
    model *= glm::translate(glm::mat4(1.0f), {0.5f, 0.5f, 0.5f});
    model *= glm::eulerAngleXYZ(glm::radians(r), glm::radians(0.0f), glm::radians(r));
    model *= glm::translate(glm::mat4(1.0f), {-0.5f, -0.5f, -0.5f});

    glm::mat4 view{1.0f};
    glm::vec2 padding = pixel_size * 90.0f;
    glm::vec2 size = pixel_size * 400.0f;
    glm::vec2 sxsy = glm::vec2(size.x - 2.0f * padding.x, 2.0f * padding.y - size.y);
    glm::vec2 tt = glm::vec2(1.0f + padding.x - size.x, -1.0f - padding.y + size.y);
    view[0][0] = sxsy.x;
    view[1][1] = sxsy.y;
    view[3][0] = tt.x;
    view[3][1] = tt.y;

    glm::mat4 camera_matrix = proj * view * model;

    shader.use();
    atlas_texture.bind(0);
    shader.set_uniform_mat4("camera_matrix", camera_matrix);
    shader.set_uniform_float("light_dir", light.x, light.y, light.z);
    shader.set_uniform_float("camera_pos", 0.0f, 0.0f, 0.0f);
    shader.set_uniform_float("alpha", 1.0f);

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
  }

private:
  void update_mesh(CubeId cube) {
    vertices.clear();
    cubes_get(cube).get_vertices(CubePos{}, StaticRandom::get().next<i32>(), rgb{90, 132, 41}, vertices);
    // Create and bind VAO
    glGenVertexArrays(1, (GLuint*)&vao);

    glBindVertexArray(vao);

    // Create vertex VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CompactVertex), vertices.data(), GL_STATIC_DRAW);

    // Bind vertex position
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pos));

    // Bind packed normals information
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, pack));

    // Bind foliage color
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(CompactVertex), (void*)offsetof(CompactVertex, foliage_color));

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
};