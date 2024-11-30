#define GLM_FORCE_RADIANS
#include "common.hpp"
#include "glad/glad.h"

#define STBI_ASSERT(x) ensure(x);
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unistd.h>
#include "cube_indicator_renderer.hpp"
#include "held_cube_renderer.hpp"
#include "input.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

void check_opengl_errors() {
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    std::stringstream error_hex;
    error_hex << std::hex << error;
    debug_warn("GL error 0x", error_hex.str());
  }
}

int main() {
  std::cout << std::setprecision(2) << std::fixed << std::showpoint << std::boolalpha;
  if (!glfwInit()) {
    debug_error("Failed to initialize glfw");
  }

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Kotník", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetWindowSizeCallback(window, []([[maybe_unused]] GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
  });
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowSize(window, mode->width, mode->height);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // // Setup GLAD
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // Configure OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);

  Texture atlas_texture{"atlas.png"};

  Shader cube_shader{"cube"};

  CubeIndicatorRenderer cube_indicator_renderer;

  auto held_cube_renderer = HeldCubeRenderer(cube_shader, atlas_texture);

  Shader crosshair_shader{"crosshair"};

  Texture crosshair_texture{"crosshair.png"};

  // Create crosshair VAO
  GLuint crosshair_vao;
  glGenVertexArrays(1, &crosshair_vao);
  glBindVertexArray(crosshair_vao);

  GLuint crosshair_vbo;
  // clang-format off
  std::vector<glm::vec<2, float>> crosshair_vertices = {
      {-1.0f, -1.0f}, {0.0f, 0.0f},
      {1.0f, -1.0f}, {1.0f, 0.0f},
      {-1.0f, 1.0f}, {0.0f, 1.0f},
      {1.0f, 1.0f}, {1.0f, 1.0f},
  };
  // clang-format on
  glGenBuffers(1, &crosshair_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo);
  glBufferData(GL_ARRAY_BUFFER, crosshair_vertices.size() * sizeof(glm::vec<2, float>), crosshair_vertices.data(), GL_STATIC_DRAW);
  // Vertex
  glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec<2, float>), (void*)0);
  // UV
  glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec<2, float>), (void*)sizeof(glm::vec<2, float>));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  World world;
  WorldRenderer world_renderer(world, cube_shader, atlas_texture);
  world.add_entity<Player>({0, 20, 0});

  Input::init(window);

  while (!glfwWindowShouldClose(window)) {
    auto start_time = std::chrono::high_resolution_clock::now();

    check_opengl_errors();

    Input::update();
    world.update();

    const Player* player = world.get_player();

    glm::mat4 camera_matrix = glm::mat4(1.0);
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    float aspect_ratio = window_width / (float)window_height;
    camera_matrix *= glm::perspective(glm::radians<float>(75.0), aspect_ratio, 0.01f, 1000.0f);
    camera_matrix *= glm::lookAt(
        static_cast<glm::vec<3, float>>(player->camera_offset),
        static_cast<glm::vec<3, float>>(player->camera_offset + player->get_looking_dir()),
        {0.0f, 1.0f, 0.0f});

    auto camera_pos = player->world_pos;

    // Clear
    glClearColor(0.59f, 0.83f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw cubes
    world_renderer.update(camera_pos, camera_matrix);

    CubeId players_held_cube = player ? player->cube_to_place : CubeId::AIR;
    held_cube_renderer.draw(aspect_ratio, players_held_cube);

    // Draw cube indicator
    std::optional<CubePos> cube_indicator_pos = player->get_cube_indicator_pos();
    if (cube_indicator_pos.has_value()) {
      cube_indicator_renderer.draw(camera_pos, camera_matrix, cube_indicator_pos.value());
    }

    // Draw crosshair
    crosshair_shader.use();
    crosshair_shader.set_uniform_float("aspect_ratio", aspect_ratio);
    glBindVertexArray(crosshair_vao);
    crosshair_texture.bind(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Swap the front and back buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

    // Delta
    auto end_time = std::chrono::high_resolution_clock::now();
    uint16_t delta = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    usleep(std::max(delta - 16666, 0));
  }

  Benchmark::print_all();

  // glfwTerminate();
  return 0;
}