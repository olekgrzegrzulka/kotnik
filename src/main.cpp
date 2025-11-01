
#include "debug.hpp"
#define GLM_FORCE_RADIANS

#include "glad/glad.h"

#include "common.hpp"
#define STBI_ASSERT(x) ensure(x);
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <unistd.h>
#include "biome.hpp"
#include "clouds.hpp"
#include "cube_indicator_renderer.hpp"
#include "cubes.hpp"
#include "framebuffer.hpp"
#include "held_cube_renderer.hpp"
#include "input.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "skybox.hpp"
#include "texture.hpp"
#include "ui/crosshair.hpp"
#include "ui/debug_panel.hpp"
#include "ui/label.hpp"
#include "ui/pause_menu.hpp"
#include "ui/ui.hpp"
#include "water_overlay.hpp"
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
  GLFWwindow* window = glfwCreateWindow(800, 600, "Kotník", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &window_size);
  glfwSetWindowSizeCallback(window, []([[maybe_unused]] GLFWwindow* window_, int width, int height) {
    glViewport(0, 0, width, height);
    auto* window_size_ = reinterpret_cast<glm::vec<2, i32>*>(glfwGetWindowUserPointer(window_));
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
  Texture atlas_texture_foliage_mask{"atlas_foliage_mask.png"};

  Shader cube_shader{"cube"};

  CubeIndicatorRenderer cube_indicator_renderer;

  auto held_cube_renderer = HeldCubeRenderer(cube_shader, atlas_texture);

  cubes_init();
  skybox_init();
  water_overlay_init();
  clouds_init();

  World world;
  WorldRenderer world_renderer(world, cube_shader, atlas_texture, atlas_texture_foliage_mask);
  world.add_entity<Player>({0, 20, 0});

  std::stringstream ss_seed;
  ss_seed << "Seed: " << world.get_seed();

  UI ui{window_size.x, window_size.y};
  [[maybe_unused]] auto& crosshair = ui.add_widget<Crosshair>();
  auto& debug_panel = ui.add_widget<DebugPanel>(ss_seed, world);
  auto& pause_menu = ui.add_widget<PauseMenu>();

  Input::init(window);
  auto framebuffer = FrameBuffer(window_size.x / 2, window_size.y / 2);
  uint16_t delta = 0;

  while (!glfwWindowShouldClose(window)) {
    auto start_time = std::chrono::high_resolution_clock::now();

    check_opengl_errors();

    Input::update();

    world.update();

    const Player* player = world.get_player();

    {
      bool pause_menu_new_visiblity = pause_menu.get_process();
      pause_menu.set_size(window_size.x, window_size.y);

      if (Input::key_just_pressed(Input::Key::KEY_ESCAPE)) {
        pause_menu_new_visiblity = !pause_menu_new_visiblity;
      }

      if (pause_menu.resume_pressed) {
        pause_menu.resume_pressed = false;
        pause_menu_new_visiblity = false;
      }

      if (pause_menu_new_visiblity != pause_menu.get_process()) {
        if (pause_menu_new_visiblity) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
      }

      pause_menu.set_process(pause_menu_new_visiblity);

      if (pause_menu.quit_pressed) {
        pause_menu.quit_pressed = false;
        break;
      }

      debug_panel.delta_ms = delta * 0.001;
    }

    glm::mat4 camera_matrix = glm::mat4(1.0);
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    static glm::vec<2, i32> prev_window_size = {0, 0};
    if (prev_window_size != window_size) {
      framebuffer.resize(window_size.x / 2, window_size.y / 2);
      prev_window_size = window_size;
    }

    float aspect_ratio = window_width / (float)window_height;
    camera_matrix *= glm::perspective(glm::radians<float>(75.0), aspect_ratio, 0.01f, 1000.0f);
    camera_matrix *= glm::lookAt(
        static_cast<glm::vec<3, float>>(player->camera_offset),
        static_cast<glm::vec<3, float>>(player->camera_offset + player->get_looking_dir()),
        {0.0f, 1.0f, 0.0f});

    auto camera_pos = player->world_pos;

    framebuffer.attach();

    glDisable(GL_DEPTH_TEST);
    skybox_draw(camera_matrix);
    clouds_draw(camera_matrix);
    glEnable(GL_DEPTH_TEST);
    world_renderer.update(camera_pos, camera_matrix);
    if (player && player->is_viewport_in_water()) {
      glDisable(GL_DEPTH_TEST);
      water_overlay_draw(camera_matrix);
    }
    CubeId players_held_cube = player ? player->cube_to_place : CubeId::AIR;
    held_cube_renderer.draw(aspect_ratio, players_held_cube);
    std::optional<CubePos> cube_indicator_pos = player->get_cube_indicator_pos();
    if (cube_indicator_pos.has_value()) {
      auto& aabbs = cubes_get(world.get_cube(cube_indicator_pos.value())).hitbox_aabbs;
      cube_indicator_renderer.draw(camera_pos, camera_matrix, cube_indicator_pos.value(), aabbs);
    }

    framebuffer.draw(window_size.x, window_size.y);

    ui.update(window_size.x, window_size.y);
    ui.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();

    // Delta
    auto end_time = std::chrono::high_resolution_clock::now();
    delta = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    usleep(std::max(delta - 16666, 0));
  }

  Benchmark::print_all();

  // glfwTerminate();
  return 0;
}