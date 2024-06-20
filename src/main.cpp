#include <iomanip>
#include <ios>
#define GLM_FORCE_RADIANS
#include "glad/glad.h"

#include <chrono>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unistd.h>
#include "cube_indicator_renderer.hpp"
#include "input.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

int main() {
  std::cout << std::setprecision(2) << std::fixed << std::showpoint << std::boolalpha;
  // Setup GLFW
  glfwInit();

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
  });
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowSize(window, mode->width, mode->height);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Setup GLAD
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // Configure OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Load textures
  GLuint texture = load_texture("assets/atlas.png");
  GLuint texture_sampler = create_sampler();

  // Create cube shader program
  std::string vertexShaderSource = read_file("shaders/cube.vert");
  std::string fragmentShaderSource = read_file("shaders/cube.frag");
  GLuint vertexShader = compile_vertex_shader(vertexShaderSource);
  GLuint fragmentShader = compile_fragment_shader(fragmentShaderSource);
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  CubeIndicatorRenderer::init();

  // Create crosshair shader program
  GLuint crosshair_vertex = compile_vertex_shader(read_file("shaders/crosshair.vert"));
  GLuint crosshair_fragment = compile_fragment_shader(read_file("shaders/crosshair.frag"));
  GLuint crosshair_program = glCreateProgram();
  glAttachShader(crosshair_program, crosshair_vertex);
  glAttachShader(crosshair_program, crosshair_fragment);
  glLinkProgram(crosshair_program);

  // Create crosshair texture and sampler
  GLuint crosshair_texture = load_texture("assets/crosshair.png");
  GLuint crosshair_sampler;
  glCreateSamplers(1, &crosshair_sampler);
  glSamplerParameteri(crosshair_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(crosshair_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(crosshair_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(crosshair_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
  WorldRenderer world_renderer(world);
  world.add_entity<Player>({1500, 60, 800});

  Input::init(window);

  while (!glfwWindowShouldClose(window)) {
    auto start_time = std::chrono::high_resolution_clock::now();

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
    glClearColor(0.65f, 0.9f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw cubes
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, texture_sampler);
    world_renderer.update(camera_pos, camera_matrix);

    // Draw cube indicator
    std::optional<CubePos> cube_indicator_pos = player->get_cube_indicator_pos();
    if (cube_indicator_pos.has_value()) {
      CubeIndicatorRenderer::draw(camera_pos, camera_matrix, cube_indicator_pos.value());
    }

    // Draw crosshair
    glUseProgram(crosshair_program);
    glBindVertexArray(crosshair_vao);
    glBindSampler(0, crosshair_sampler);
    glBindTextureUnit(0, crosshair_texture);
    glUniform1f(0, aspect_ratio);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Swap the front and back buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

    // Delta
    auto end_time = std::chrono::high_resolution_clock::now();
    uint16_t delta = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    usleep(std::max(delta - 16666, 0));
  }

  // Clean up and exit
  glDeleteProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glfwTerminate();
  return 0;
}