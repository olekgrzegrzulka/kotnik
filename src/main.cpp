#define GLM_FORCE_RADIANS
#include "glad/glad.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unistd.h>
#include "chunk_renderer.hpp"
#include "input.hpp"
#include "player.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "world.hpp"

glm::mat4 matrix{1.0f};

static std::string read_file(const std::string& filename) {
  std::ifstream file(filename, std::ios::in);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file '" + filename + "'!");
  }
  std::string source;
  source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  return source;
}

int main() {
  std::cout << std::setprecision(2) << std::boolalpha;
  // Setup GLFW
  glfwInit();

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle", NULL, NULL);
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

  // Create cube indicator shader program
  GLuint cube_indicator_vertex = compile_vertex_shader(read_file("shaders/cube_indicator.vert"));
  GLuint cube_indicator_fragment = compile_fragment_shader(read_file("shaders/cube_indicator.frag"));
  GLuint cube_indicator_program = glCreateProgram();
  glAttachShader(cube_indicator_program, cube_indicator_vertex);
  glAttachShader(cube_indicator_program, cube_indicator_fragment);
  glLinkProgram(cube_indicator_program);

  // Create cube indicator VAO
  GLuint cube_indicator_vao;
  glGenVertexArrays(1, &cube_indicator_vao);
  glBindVertexArray(cube_indicator_vao);

  GLuint cube_indicator_vbo;
  std::vector<WorldPos> cube_indicator_vertices = {
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
  glBufferData(GL_ARRAY_BUFFER, cube_indicator_vertices.size() * sizeof(WorldPos), cube_indicator_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, cube_indicator_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WorldPos), (void*)0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

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
  world.add_entity<Player>({-10000, 50, 0});

  Input::init(window);

  while (!glfwWindowShouldClose(window)) {
    auto start_time = std::chrono::high_resolution_clock::now();

    Input::update();
    world.update();

    const Player* player = world.get_player();

    matrix = glm::mat4(1.0);

    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    float aspect_ratio = window_width / (float)window_height;
    matrix *= glm::perspective(glm::radians<float>(75.0), aspect_ratio, 0.01f, 1000.0f);
    matrix *= glm::lookAt(player->camera_offset, player->camera_offset + player->get_looking_dir(), {0.0f, 1.0f, 0.0f});

    auto player_pos = player->world_pos;

    // -----------------
    //     Rendering
    // -----------------

    // Clear
    glClearColor(0.65f, 0.9f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw cubes
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, texture_sampler);
    world.draw(player_pos, matrix);

    // Draw cube indicator
    std::optional<CubePos> cube_indicator_pos = player->get_cube_indicator_pos();
    if (cube_indicator_pos.has_value()) {
      glUseProgram(cube_indicator_program);
      glBindVertexArray(cube_indicator_vao);
      glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(matrix));
      glUniform3f(1, (float)(cube_indicator_pos.value().x - player_pos.x), (float)(cube_indicator_pos.value().y - player_pos.y), (float)(cube_indicator_pos.value().z - player_pos.z));
      glLineWidth(2.5f);
      glDrawArrays(GL_LINE_STRIP, 0, cube_indicator_vertices.size());
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