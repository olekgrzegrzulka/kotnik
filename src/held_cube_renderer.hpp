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
#include "cubes.hpp"
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

  void draw(float aspect_ratio, CubeId new_cube_id) {
    if (!cube_id.has_value() || cube_id.value() != new_cube_id) {
      cube_id = new_cube_id;
      update_mesh(cube_id.value());
    }

    shader.use();
    atlas_texture.bind(0);

    static constexpr glm::vec3 light = {0.41f, 0.82f, 0.41f};

    glm::mat4 projection = glm::ortho(-0.5f * aspect_ratio, 0.5f * aspect_ratio, -0.5f, 0.5f, 0.01f, 10.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec<3, float>{1.0f, 0.0f, 0.0f},
        glm::vec<3, float>{0.0f},
        {0.0f, 1.0f, 0.0f});

    view *= glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -0.41f, -0.82f));
    view *= glm::scale(glm::mat4(1.0), glm::vec3(0.1f));

    auto now = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    float r = static_cast<float>((t / 32) % 360);

    glm::mat4 model = glm::mat4(1.0f);
    model *= glm::eulerAngleXYZ(glm::radians(0.0f), glm::radians(r), glm::radians(r));
    model *= glm::translate(glm::mat4(1.0f), {-0.5f, -0.5f, -0.5f});

    glm::mat4 camera_matrix = projection * view * model;

    glDisable(GL_DEPTH_TEST);

    shader.set_uniform_mat4("camera_matrix", camera_matrix);
    shader.set_uniform_float("light_dir", light.x, light.y, light.z);
    shader.set_uniform_float("camera_pos", 0.0f, 0.0f, 0.0f);
    shader.set_uniform_float("alpha", 1.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
  }

private:
  void update_mesh(CubeId cube) {
    vertices.clear();
    cubes_get(cube).get_vertices(CubePos{}, StaticRandom::get().next<i32>(), vertices);
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

    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
};