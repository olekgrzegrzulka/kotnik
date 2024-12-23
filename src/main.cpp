#include "ui/sprite.hpp"
#include "ui/widget.hpp"
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
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unistd.h>
#include "clouds.hpp"
#include "cube_indicator_renderer.hpp"
#include "cubes.hpp"
#include "held_cube_renderer.hpp"
#include "input.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "skybox.hpp"
#include "texture.hpp"
#include "ui/ui.hpp"
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

  glm::vec<2, i32> window_size;

  // glfw
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Kotník", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &window_size);
  glfwSetWindowSizeCallback(window, []([[maybe_unused]] GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    auto* window_size_ = reinterpret_cast<glm::vec<2, i32>*>(glfwGetWindowUserPointer(window));
    window_size_->x = width;
    window_size_->y = height;
  });
  glfwGetWindowSize(window, &window_size.x, &window_size.y);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // gl
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_BACK);
  glViewport(0, 0, window_size.x, window_size.y);

  Texture atlas_texture{"atlas.png"};

  Shader cube_shader{"cube"};

  CubeIndicatorRenderer cube_indicator_renderer;

  auto held_cube_renderer = HeldCubeRenderer(cube_shader, atlas_texture);

  cubes_init();
  skybox_init();
  clouds_init();

  World world;
  WorldRenderer world_renderer(world, cube_shader, atlas_texture);
  world.add_entity<Player>({0, 20, 0});

  UI ui{window_size.x, window_size.y};
  auto& crosshair = ui.add_widget<Sprite>();
  crosshair.set_width(32);
  crosshair.set_height(32);
  crosshair.set_uv_start({0.0 / 16.0, 6.0 / 16.0});
  crosshair.set_uv_end({1.0 / 16.0, 7.0 / 16.0});
  crosshair.set_anchor(Anchor::CENTER_CENTER);
  crosshair.set_screen_anchor(Anchor::CENTER_CENTER);

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

    glDisable(GL_DEPTH_TEST);
    skybox_draw(camera_matrix);
    clouds_draw(camera_matrix);

    // Draw cubes
    glEnable(GL_DEPTH_TEST);
    world_renderer.update(camera_pos, camera_matrix);

    CubeId players_held_cube = player ? player->cube_to_place : CubeId::AIR;
    held_cube_renderer.draw(aspect_ratio, players_held_cube);

    // Draw cube indicator
    std::optional<CubePos> cube_indicator_pos = player->get_cube_indicator_pos();
    if (cube_indicator_pos.has_value()) {
      cube_indicator_renderer.draw(camera_pos, camera_matrix, cube_indicator_pos.value());
    }

    ui.update(window_size.x, window_size.y);
    ui.draw();

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