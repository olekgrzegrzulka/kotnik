
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
#include "chunk.hpp"
#include "clouds.hpp"
#include "cube_indicator_renderer.hpp"
#include "cubes.hpp"
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

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D screenTexture;
    void main() {
        FragColor = texture(screenTexture, TexCoord);
    }
)";

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

  Shader cube_shader{"cube"};

  CubeIndicatorRenderer cube_indicator_renderer;

  auto held_cube_renderer = HeldCubeRenderer(cube_shader, atlas_texture);

  cubes_init();
  skybox_init();
  water_overlay_init();
  clouds_init();

  World world;
  WorldRenderer world_renderer(world, cube_shader, atlas_texture);
  world.add_entity<Player>({0, 20, 0});

  std::stringstream ss_seed;
  ss_seed << "Seed: " << world.get_seed();

  UI ui{window_size.x, window_size.y};
  [[maybe_unused]] auto& crosshair = ui.add_widget<Crosshair>();
  auto& debug_panel = ui.add_widget<DebugPanel>(ss_seed);
  auto& pause_menu = ui.add_widget<PauseMenu>();

  Input::init(window);

  // Create framebuffer
  GLuint fbo, texture, depthRBO;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Create color texture for framebuffer
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_size.x / 2, window_size.y / 2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  // Create depth renderbuffer
  glGenRenderbuffers(1, &depthRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_size.x / 2, window_size.y / 2);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    debug_error("failed to create framebuffer");
  }

  float quadVertices[] = {
      -1.0f, -1.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 0.0f, 1.0f};
  GLuint quadIndices[] = {0, 1, 2, 2, 3, 0};
  GLuint vbo, vao, ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  Shader framebuffer_shader = Shader{"framebuffer"};

  GLuint framebuffer_sampler = 0;
  glCreateSamplers(1, &framebuffer_sampler);
  glSamplerParameteri(framebuffer_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(framebuffer_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(framebuffer_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(framebuffer_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  uint16_t delta = 0;
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

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, window_size.x / 2, window_size.y / 2);
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

    if (player && player->is_viewport_in_water()) {
      glDisable(GL_DEPTH_TEST);
      water_overlay_draw(camera_matrix);
    }

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

    auto [player_chunk_pos, player_local_pos] = cube_to_local(player->world_pos);
    Chunk* player_chunk = world.get_chunk(player_chunk_pos);

    std::stringstream ss_frametime_text;
    ss_frametime_text << std::fixed << std::setprecision(2);
    ss_frametime_text << "Frametime: " << delta * 0.001 << " ms";
    std::string frametime_text = ss_frametime_text.str();

    debug_panel.label_frametime.set_text(frametime_text);
    debug_panel.label_frametime_shadow.set_text(frametime_text);

    std::stringstream ss_player_pos_text;
    ss_player_pos_text << std::fixed << std::setprecision(2);
    ss_player_pos_text << "World: [" << player->world_pos.x << ", " << player->world_pos.y << ", " << player->world_pos.z << "]";
    std::string player_pos_text = ss_player_pos_text.str();

    debug_panel.label_player_pos.set_text(player_pos_text);
    debug_panel.label_player_pos_shadow.set_text(player_pos_text);

    std::stringstream ss_player_chunk_pos_text;
    ss_player_chunk_pos_text << std::fixed << std::setprecision(2);
    ss_player_chunk_pos_text << "Chunk: [" << player_chunk_pos.x << ", " << player_chunk_pos.y << ", " << player_chunk_pos.z << "]";
    std::string player_chunk_pos_text = ss_player_chunk_pos_text.str();

    debug_panel.label_player_chunk_pos.set_text(player_chunk_pos_text);
    debug_panel.label_player_chunk_pos_shadow.set_text(player_chunk_pos_text);
    static i32 get_biome_counter = 0;
    static auto get_biome_last_player_pos = player->world_pos;
    if (get_biome_counter-- <= 0) {
      if ((get_biome_last_player_pos - player->world_pos).length() > 0.5) {
        get_biome_last_player_pos = player->world_pos;
        auto biome_name = "Biome: " + world.get_world_gen().get_blended_biome(player->world_pos).name;
        debug_panel.label_biome_name.set_text(biome_name);
        debug_panel.label_biome_name_shadow.set_text(biome_name);
      }
      get_biome_counter = 10;
    }

    if (player_chunk) {
      std::stringstream ss_lightmap_text;
      ss_lightmap_text << "Lightmap: " << (i32)player_chunk->get_lightmap(player_local_pos);
      debug_panel.label_lightmap.set_text(ss_lightmap_text.str());
      debug_panel.label_lightmap_shadow.set_text(ss_lightmap_text.str());
    } else {
      debug_panel.label_lightmap.set_text("Lightmap: ?");
      debug_panel.label_lightmap_shadow.set_text("Lightmap: ?");
    }

    auto heightmap = player_chunk ? player_chunk->get_heightmap(player_local_pos.x, player_local_pos.z) : std::nullopt;
    if (heightmap.has_value()) {
      std::stringstream ss_heightmap_text;
      ss_heightmap_text << "Heightmap: " << (i32)heightmap.value() + player_chunk_pos.y * Chunk::chunk_size;
      debug_panel.label_heightmap.set_text(ss_heightmap_text.str());
      debug_panel.label_heightmap_shadow.set_text(ss_heightmap_text.str());
    } else {
      debug_panel.label_heightmap.set_text("Heightmap: ?");
      debug_panel.label_heightmap_shadow.set_text("Heightmap: ?");
    }

    // Render framebuffer texture to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_size.x, window_size.y);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindSampler(0, framebuffer_sampler);
    framebuffer_shader.use();
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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